





#include "CATextView.h"
#include "basics/CAApplication.h"
#include "view/CAWindow.h"
#include "actions/CCActionInterval.h"
#include "CCEGLView.h"
#include <utility>

#if (CC_TARGET_PLATFORM != CC_PLATFORM_LINUX)
#include "platform/CAFTFontCache.h"
#endif

NS_CC_BEGIN

#pragma CAListView

CATextView::CATextView()
: m_pTextViewDelegate(NULL)
, m_pBackgroundView(NULL)
, m_pCursorMark(NULL)
, m_pImageView(NULL)
, m_cCursorColor(CAColor_black)
, m_szFontName("")
, m_iFontSize(24)
, m_iCurPos(0)
{
	m_iLineHeight = CAImage::getFontHeight(m_szFontName.c_str(), m_iFontSize);
}


CATextView::~CATextView()
{
    m_pTextViewDelegate = NULL;
}


bool CATextView::resignFirstResponder()
{
	bool result = CAView::resignFirstResponder();
	if (result)
	{
		detachWithIME();
	}
	return result;
}

bool CATextView::becomeFirstResponder()
{
	bool result = CAView::becomeFirstResponder();
	if (result)
	{
		attachWithIME();
	}
	return result;
}

CATextView* CATextView::createWithFrame(const CCRect& rect)
{
	CATextView* pTextView = new CATextView();
	if (pTextView && pTextView->initWithFrame(rect))
	{
		pTextView->autorelease();
		return pTextView;
	}
	CC_SAFE_DELETE(pTextView);
	return NULL;
}

CATextView* CATextView::createWithCenter(const CCRect& rect)
{
	CATextView* pTextView = new CATextView();
	if (pTextView && pTextView->initWithCenter(rect))
	{
		pTextView->autorelease();
		return pTextView;
	}
	CC_SAFE_DELETE(pTextView);
	return NULL;
}

bool CATextView::init()
{
	if (!CAScrollView::init())
	{
		return false;
	}

	this->setShowsHorizontalScrollIndicator(false);
	this->setBounceHorizontal(false);
	this->setTouchMovedListenHorizontal(false);

	m_pImageView = new CAImageView();
	this->addSubview(m_pImageView);
	return true;
}

bool CATextView::initWithFrame(const CCRect& frame)
{
	if (!CAScrollView::initWithFrame(frame, CAColor_red))
	{
		return false;
	}
	initMarkSprite();
	return true;
}

bool CATextView::initWithCenter(const CCRect& rect)
{
	if (!CAScrollView::initWithCenter(rect))
	{
		return false;
	}
	initMarkSprite();
	return true;
}

void CATextView::initMarkSprite()
{
	CC_SAFE_DELETE(m_pCursorMark);
	m_pCursorMark = CAView::create();
	m_pCursorMark->setColor(m_cCursorColor);
	m_pCursorMark->setVisible(false);

	m_pCursorMark->setFrame(CCRect(0, 0, 2, m_iLineHeight));
	addSubview(m_pCursorMark);
	m_pCursorMark->runAction(CCRepeatForever::create((CCActionInterval *)CCSequence::create(CCFadeOut::create(0.5f), CCFadeIn::create(0.5f), NULL)));
}

void CATextView::setBackGroundImage(CAImage *image)
{
	if (m_pBackgroundView == NULL)
	{
		m_pBackgroundView = CAScale9ImageView::create();
		m_pBackgroundView->setFrame(this->getBounds());
		this->insertSubview(m_pBackgroundView, -1);
	}
	m_pBackgroundView->setImage(image);
}

CAImage *CATextView::getBackGroundImage()
{
	return m_pBackgroundView ? m_pBackgroundView->getImage() : NULL;
}

void CATextView::updateImage()
{
	float width = this->getBounds().size.width;
	float height = this->getBounds().size.height;
	CCSize size = CCSizeMake(width, 0);

	CAImage* image = NULL;
#if (CC_TARGET_PLATFORM != CC_PLATFORM_LINUX)

	image = g_AFTFontCache.initWithStringEx(m_szText.c_str(),
		m_szFontName.c_str(),
		m_iFontSize,
		width,
		0,
		m_vLinesTextView);
#endif

	if (image == NULL)
	{
		m_vLinesTextView.clear();
	}

	m_pImageView->initWithImage(image);
	setViewSize(m_pImageView->getBounds().size);
	calcCursorPosition();
}


void CATextView::calcCursorPosition()
{
	int iCurLine = -1, iCurLineCharPos = 0;

	for (int i = 0; i < m_vLinesTextView.size(); i++)
	{
		TextViewLineInfo& t = m_vLinesTextView[i];

		if (m_iCurPos >= t.iStartCharPos && m_iCurPos <= t.iEndCharPos)
		{
			iCurLine = i; iCurLineCharPos = t.iStartCharPos;
			break;
		}
	}

	float fHalfLineHeight = m_iLineHeight / 2.0f;

	CCPoint cCurPosition;
	if (iCurLine == -1)
	{
		cCurPosition.x = 0;
		cCurPosition.y = fHalfLineHeight;
	}
	else
	{
		std::string s = m_szText.substr(iCurLineCharPos, m_iCurPos - iCurLineCharPos);
		if (!s.empty() && s[0] == '\n')
		{
			s.erase(0, 1);
		}
		cCurPosition.x = getStringLength(s);
		cCurPosition.y = (iCurLine * 2 + 1) * fHalfLineHeight;
	}

	if (m_pCursorMark)
	{
		m_pCursorMark->setCenterOrigin(cCurPosition);
	}

	float w = getBounds().size.height;
	float y = cCurPosition.y - getContentOffset().y;
	if (y < 0 || y > w)
	{
		y = y < 0 ? fHalfLineHeight : w - fHalfLineHeight;
		setContentOffset(CCPointMake(0, cCurPosition.y-y), true);
	}
}


void CATextView::setFontSize(int var)
{
	m_iFontSize = var;
	m_iLineHeight = CAImage::getFontHeight(m_szFontName.c_str(), m_iFontSize);
	initMarkSprite();
	updateImage();
}

int CATextView::getFontSize()
{
	return m_iFontSize;
}

void CATextView::setFontName(const std::string& var)
{
	m_szFontName = var;
	m_iLineHeight = CAImage::getFontHeight(m_szFontName.c_str(), m_iFontSize);
	initMarkSprite();
	updateImage();
}

const std::string& CATextView::getFontName()
{
	return m_szFontName;
}

void CATextView::setCursorColor(CAColor4B var)
{
	m_cCursorColor = var;
	if (m_pCursorMark)
	{
		m_pCursorMark->setColor(m_cCursorColor);
	}
}

CAColor4B CATextView::getCursorColor()
{
	return m_cCursorColor;
}

bool CATextView::canAttachWithIME()
{
	return (m_pTextViewDelegate) ? m_pTextViewDelegate->onTextViewAttachWithIME(this) : true;
}

bool CATextView::canDetachWithIME()
{
	return (m_pTextViewDelegate) ? m_pTextViewDelegate->onTextViewDetachWithIME(this) : true;
}

void CATextView::insertText(const char * text, int len)
{
	m_szText.insert(m_iCurPos, text, len);
 	m_iCurPos += len;
	updateImage();
}

void CATextView::willInsertText(const char* text, int len)
{

}

void CATextView::AndroidWillInsertText(int start, const char* str, int before, int count)
{

}

void CATextView::deleteBackward()
{
	if (m_iCurPos == 0 || m_szText.empty())
		return;

	CC_RETURN_IF(m_pTextViewDelegate && m_pTextViewDelegate->onTextViewDeleteBackward(this, m_szText.c_str(), m_szText.length()));

	int nDeleteLen = 1;
	while (0x80 == (0xC0 & m_szText.at(m_iCurPos - nDeleteLen)))
	{
		++nDeleteLen;
	}

	m_szText.erase(m_iCurPos - nDeleteLen, nDeleteLen);
	m_iCurPos -= nDeleteLen;
	updateImage();
}

float CATextView::maxSpeed(float dt)
{
    return (CCPoint(m_obContentSize).getLength() * 8 * dt);
}

float CATextView::maxSpeedCache(float dt)
{
    return (maxSpeed(dt) * 3.0f);
}

float CATextView::decelerationRatio(float dt)
{
    return 2.0f * dt;
}

bool CATextView::ccTouchBegan(CATouch *pTouch, CAEvent *pEvent)
{
	if (m_pTouches->count() > 0)
	{
		m_pTouches->replaceObjectAtIndex(0, pTouch);
		return true;
	}
	bool isInertia = m_tInertia.getLength() < 1.0f;
	if (!CAScrollView::ccTouchBegan(pTouch, pEvent))
		return false;

	CCPoint point = this->convertTouchToNodeSpace(pTouch);

	if (this->getBounds().containsPoint(point))
	{
		becomeFirstResponder();
		if (isFirstResponder())
		{
			m_pCursorMark->setVisible(true);

			point.y += getContentOffset().y;
			int iCurLine = point.y / m_iLineHeight;
			if (m_vLinesTextView.empty())
			{
				iCurLine = 0;
			}
			else if (iCurLine >= m_vLinesTextView.size())
			{
				iCurLine = m_vLinesTextView.size() - 1;
			}
			
			int iHalfCharSize = 0;
			int iStartPos = 0;
			if (!m_vLinesTextView.empty())
			{
				m_iCurPos = iStartPos = m_vLinesTextView[iCurLine].iStartCharPos;
				std::vector<TextAttribute>& v = m_vLinesTextView[iCurLine].TextAttrVect;
				for (int i = 0, iStringLeftX = 0; i < v.size(); i++)
				{
					TextAttribute& t = v[i];
					if (point.x >= iStringLeftX - iHalfCharSize && point.x < iStringLeftX + t.charlength / 2)
					{
						break;
					}

					iStringLeftX += t.charlength;
					m_iCurPos += t.charSize;
					iHalfCharSize = t.charlength / 2;
				}
			}

			std::string s = m_szText.substr(iStartPos, m_iCurPos - iStartPos);
			if (!s.empty() && s[0] == '\n')
			{
				s.erase(0, 1);
			}
			m_pCursorMark->setCenterOrigin(CCPoint(getStringLength(s), (iCurLine * 2 + 1) * (m_iLineHeight / 2)));
		}
		return true;
	}
	else
	{
		if (resignFirstResponder())
		{
			m_pCursorMark->setVisible(false);
			return false;
		}
		return false;
	}

	return true;
}

bool CATextView::attachWithIME()
{
	bool bRet = CAIMEDelegate::attachWithIME();
	if (bRet)
	{
		// open keyboard
		CCEGLView * pGlView = CAApplication::getApplication()->getOpenGLView();
		if (pGlView)
		{
#if(CC_TARGET_PLATFORM==CC_PLATFORM_ANDROID||CC_TARGET_PLATFORM==CC_PLATFORM_IOS)        
			if (getKeyboardType() == KEY_BOARD_TYPE_NORMAL)
			{
				pGlView->setIMEKeyboardDefault();
			}
			else if (getKeyboardType() == KEY_BOARD_TYPE_NUMBER)
			{
				pGlView->setIMEKeyboardNumber();
			}
			else if (getKeyboardType() == KEY_BOARD_TYPE_ALPHABET)
			{
				pGlView->setIMEKeyboardAlphabet();
			}
#endif
			pGlView->setIMEKeyboardState(true);
		}
	}
	return bRet;
}

bool CATextView::detachWithIME()
{
	bool bRet = CAIMEDelegate::detachWithIME();
	if (bRet)
	{
		// close keyboard
		CCEGLView * pGlView = CAApplication::getApplication()->getOpenGLView();
		if (pGlView)
		{
			pGlView->setIMEKeyboardState(false);
		}
	}
	return bRet;
}

int CATextView::getStringLength(const std::string &var)
{
#if (CC_TARGET_PLATFORM != CC_PLATFORM_LINUX)
	return g_AFTFontCache.getStringWidth(m_szFontName.c_str(), m_iFontSize, var);
#else
	CAImage *image = CAImage::createWithString(var.c_str(),
		"",
		m_iFontSize,
		CCSizeZero,
		CATextAlignmentLeft,
		CAVerticalTextAlignmentCenter);
	if (image != NULL)
	{
		return image->getContentSize().width;
	}
	return 0;
#endif
}

NS_CC_END