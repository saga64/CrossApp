/****************************************************************************
 Copyright (c) 2010-2012 cocos2d-x.org
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#ifndef __CAHTTP_REQUEST_H__
#define __CAHTTP_REQUEST_H__

#include "CrossApp.h"
#include "ExtensionMacros.h"

NS_CC_EXT_BEGIN

class CAHttpClient;
class CAHttpResponse;

typedef void (CAObject::*SEL_HttpRsp)(CAHttpClient* client, CAHttpResponse* response);
#define httprsp_selector(_SELECTOR) (CrossApp::extension::SEL_HttpRsp)(&_SELECTOR)

/** 
 @brief defines the object which users must packed for CAHttpClient::send(HttpRequest*) method.
 Please refer to samples/TestCpp/Classes/ExtensionTest/NetworkTest/HttpClientTest.cpp as a sample
 @since v2.0.2
 @js NA
 @lua NA
 */

class CAHttpRequest : public CAObject
{
public:
    /** Use this enum type as param in setReqeustType(param) */
    typedef enum
    {
        kHttpGet,
        kHttpPost,
        kHttpPut,
        kHttpDelete,
        kHttpUnkown,
    } HttpRequestType;
    
    /** Constructor 
        Because HttpRequest object will be used between UI thead and network thread,
        requestObj->autorelease() is forbidden to avoid crashes in CCAutoreleasePool
        new/retain/release still works, which means you need to release it manually
        Please refer to HttpRequestTest.cpp to find its usage
     */
    CAHttpRequest()
    {
        _requestType = kHttpUnkown;
        _url.clear();
        _requestData.clear();
        _tag.clear();
        _pTarget = NULL;
        _pSelector = NULL;
        _pUserData = NULL;
    };
    
    /** Destructor */
    virtual ~CAHttpRequest()
    {
        if (_pTarget)
        {
            _pTarget->release();
        }
    };
    
    /** Override autorelease method to avoid developers to call it */
    CAObject* autorelease(void)
    {
        CCAssert(false, "HttpResponse is used between network thread and ui thread \
                 therefore, autorelease is forbidden here");
        return NULL;
    }
            
    // setter/getters for properties
     
    /** Required field for HttpRequest object before being sent.
        kHttpGet & kHttpPost is currently supported
     */
    inline void setRequestType(HttpRequestType type)
    {
        _requestType = type;
    };
    /** Get back the kHttpGet/Post/... enum value */
    inline HttpRequestType getRequestType()
    {
        return _requestType;
    };
    
    /** Required field for HttpRequest object before being sent.
     */
    inline void setUrl(const char* url)
    {
        _url = url;
    };
    /** Get back the setted url */
    inline const char* getUrl()
    {
        return _url.c_str();
    };
    
    /** Option field. You can set your post data here
     */
    inline void setRequestData(const char* buffer, unsigned int len)
    {
        _requestData.assign(buffer, buffer + len);
    };
    /** Get the request data pointer back */
    inline char* getRequestData()
    {
        return &(_requestData.front());
    }
    /** Get the size of request data back */
    inline int getRequestDataSize()
    {
        return _requestData.size();
    }
    
    /** Option field. You can set a string tag to identify your request, this tag can be found in HttpResponse->getHttpRequest->getTag()
     */
    inline void setTag(const char* tag)
    {
        _tag = tag;
    };
    /** Get the string tag back to identify the request. 
        The best practice is to use it in your MyClass::onMyHttpRequestCompleted(sender, HttpResponse*) callback
     */
    inline const char* getTag()
    {
        return _tag.c_str();
    };
    
    /** Option field. You can attach a customed data in each request, and get it back in response callback.
        But you need to new/delete the data pointer manully
     */
    inline void setUserData(void* pUserData)
    {
        _pUserData = pUserData;
    };
    /** Get the pre-setted custom data pointer back.
        Don't forget to delete it. HttpClient/HttpResponse/HttpRequest will do nothing with this pointer
     */
    inline void* getUserData()
    {
        return _pUserData;
    };
    
    /** Required field. You should set the callback selector function at ack the http request completed
     */
    CC_DEPRECATED_ATTRIBUTE inline void setResponseCallback(CAObject* pTarget, SEL_CallFuncND pSelector)
    {
        setResponseCallback(pTarget, (SEL_HttpRsp) pSelector);
    }

    inline void setResponseCallback(CAObject* pTarget, SEL_HttpRsp pSelector)
    {
        _pTarget = pTarget;
        _pSelector = pSelector;
        
        if (_pTarget)
        {
            _pTarget->retain();
        }
    }    
    /** Get the target of callback selector funtion, mainly used by CAHttpClient */
    inline CAObject* getTarget()
    {
        return _pTarget;
    }

    /* This sub class is just for migration SEL_CallFuncND to SEL_HttpRsp, 
       someday this way will be removed */
    class _prxy
    {
    public:
        _prxy( SEL_HttpRsp cb ) :_cb(cb) {}
        ~_prxy(){};
        operator SEL_HttpRsp() const { return _cb; }
        CC_DEPRECATED_ATTRIBUTE operator SEL_CallFuncND()   const { return (SEL_CallFuncND) _cb; }
    protected:
        SEL_HttpRsp _cb;
    };
    
    /** Get the selector function pointer, mainly used by CAHttpClient */
    inline _prxy getSelector()
    {
        return _prxy(_pSelector);
    }
    
    /** Set any custom headers **/
    inline void setHeaders(std::vector<std::string> pHeaders)
   	{
   		_headers=pHeaders;
   	}
   
    /** Get custom headers **/
   	inline std::vector<std::string> getHeaders()
   	{
   		return _headers;
   	}


protected:
    // properties
    HttpRequestType             _requestType;    /// kHttpRequestGet, kHttpRequestPost or other enums
    std::string                 _url;            /// target url that this request is sent to
    std::vector<char>           _requestData;    /// used for POST
    std::string                 _tag;            /// user defined tag, to identify different requests in response callback
    CAObject*          _pTarget;        /// callback target of pSelector function
    SEL_HttpRsp            _pSelector;      /// callback function, e.g. MyLayer::onHttpResponse(CAHttpClient *sender, CAHttpResponse * response)
    void*                       _pUserData;      /// You can add your customed data here 
    std::vector<std::string>    _headers;		      /// custom http headers
};

NS_CC_EXT_END

#endif //__CAHTTP_REQUEST_H__
