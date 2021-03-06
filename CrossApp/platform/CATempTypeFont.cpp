#include "CATempTypeFont.h"
#include "basics/CAApplication.h"
#include "platform/CCFileUtils.h"
#include "support/ccUTF8.h"


using namespace std;

NS_CC_BEGIN


CATempTypeFont::CATempTypeFont()
: m_CurFontFace(NULL)
{
}

CATempTypeFont::~CATempTypeFont()
{
	finiTempTypeFont();
}

void CATempTypeFont::initTempTypeFont(unsigned long nSize)
{
	std::map<unsigned long, FT_Face>::iterator it=m_mTempFontMap.find(nSize);
	if (it != m_mTempFontMap.end())
	{
		m_CurFontFace = it->second;
	}
	else
	{
		m_face = NULL;
		initFreeTypeFont("arial", nSize);
		m_CurFontFace = m_face;
		m_mTempFontMap[nSize] = m_CurFontFace;
	}
	m_face = NULL;
}

void CATempTypeFont::finiTempTypeFont()
{
	std::map<unsigned long, FT_Face>::iterator it = m_mTempFontMap.begin();
	while (it != m_mTempFontMap.end())
	{
		FT_Done_Face(it->second);
		it++;
	}
	m_mTempFontMap.clear();
}

NS_CC_END

