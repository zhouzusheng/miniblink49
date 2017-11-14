#include "config.h"
#include "wtf/text/TextCodecWin.h"
#include "wtf/Assertions.h"
#include "wtf/StringExtras.h"
#include "wtf/Threading.h"
#include "wtf/WTFThreadData.h"
#include "wtf/text/CString.h"
#include "wtf/text/CharacterNames.h"
#include "wtf/text/StringBuilder.h"
#include "wtf/text/WTFStringUtil.h"
#include "wtf/text/StringBuffer.h"

#include <windows.h>
#include <stdio.h>
#include <wchar.h>

using std::min;


namespace WTF {

	static CharsetInfo gCharsetInfos[MAX_CHARSET_COUNT];
	static int gCharsetCount = -1;

	void toLatin1(char* latin1, const wchar_t* name)
	{
		for (unsigned i = 0; i < MAX_NAME_LEN - 1; ++i)
		{
			wchar_t ch = name[i];
			latin1[i] = ch > 0xff ? '?' : (char)ch;

			if (ch == 0)
				break;
		}

		latin1[MAX_NAME_LEN - 1] = '\0';
	}

	void addCodePage(unsigned int codePage, const char* name)
	{
		if (name == NULL || name[0] == 0)
			return;

		CharsetInfo* pCharset = NULL;
		for (int i = 0; i < gCharsetCount; ++i)
		{
			if (gCharsetInfos[i].codePage == codePage)
			{
				pCharset = &gCharsetInfos[i];
				break;
			}
		}

		if (pCharset == NULL)
		{
			if (gCharsetCount == MAX_CHARSET_COUNT)
				return;

			CharsetInfo* pCharset = &gCharsetInfos[gCharsetCount];
			++gCharsetCount;

			pCharset->aliasCount = 0;
			pCharset->codePage = codePage;
			strcpy(pCharset->name, name);
			return;
		}

		if (pCharset->aliasCount == MAX_ALIAS_COUNT)
			return;

		if (strcmp(pCharset->name, name) == 0)
			return;

		for (unsigned int i = 0; i < pCharset->aliasCount; ++i)
		{
			if (strcmp(pCharset->aliases[i], name) == 0)
				return;
		}

		strcpy(pCharset->aliases[pCharset->aliasCount], name);
		++pCharset->aliasCount;
	}

	void addCodePage(unsigned int codePage, const wchar_t* name)
	{
		if (name == NULL || name[0] == 0)
			return;

		char latin1[MAX_NAME_LEN];
		toLatin1(latin1, name);

		addCodePage(codePage, latin1);
	}

	const CharsetInfo* getCharset(const char* name, bool alias)
	{
		for (int i = 0; i < gCharsetCount; ++i)
		{
			if (_stricmp(gCharsetInfos[i].name, name) == 0)
			{
				return &gCharsetInfos[i];
			}

			if (alias)
			{
				for (unsigned int j = 0; j < gCharsetInfos[i].aliasCount; ++j)
				{
					if (_stricmp(gCharsetInfos[i].aliases[j], name) == 0)
						return &gCharsetInfos[i];
				}
			}
		}

		return NULL;
	}

	const CharsetInfo* getCharset(unsigned int codePage)
	{
		for (int i = 0; i < gCharsetCount; ++i)
		{
			if (gCharsetInfos[i].codePage == codePage)
				return &gCharsetInfos[i];
		}

		return NULL;
	}

	void initCharsets()
	{
		static bool first = true;
		if (!first)
			return;
		first = true;

		gCharsetCount = 0;

		addCodePage(0, "NULL");

		addCodePage(10000, "macintosh");
		addCodePage(10007, "x-mac-cyrillic");

		addCodePage(936, "GBK");
		addCodePage(936, "GB2312");
		addCodePage(54936, "GB18030");
		addCodePage(950, "Big5");

		addCodePage(20866, "KOI8-R");

		addCodePage(874, "windows-874");
		addCodePage(949, "windows-949");
		addCodePage(949, "KSC_5601");
		addCodePage(1250, "windows-1250");
		addCodePage(1251, "windows-1251");
		addCodePage(1253, "windows-1253");
		addCodePage(1254, "windows-1254");
		addCodePage(1255, "windows-1255");
		addCodePage(1256, "windows-1256");
		addCodePage(1257, "windows-1257");
		addCodePage(1258, "windows-1258");

		addCodePage(864, "cp864");
		addCodePage(932, "Shift_JIS");
		addCodePage(20932, "EUC-JP");
		addCodePage(50222, "ISO-2022-JP");

		addCodePage(28591, "ISO-8859-1");
		addCodePage(28592, "ISO-8859-2");
		addCodePage(28593, "ISO-8859-3");
		addCodePage(28594, "ISO-8859-4");
		addCodePage(28595, "ISO-8859-5");
		addCodePage(28596, "ISO-8859-6");
		addCodePage(28597, "ISO-8859-7");
		addCodePage(28598, "ISO-8859-8");
		addCodePage(28599, "ISO-8859-9");
		addCodePage(28600, "ISO-8859-10");
		addCodePage(28600, "ISO-8859-13");
		addCodePage(28604, "ISO-8859-14");
		addCodePage(28605, "ISO-8859-15");
		addCodePage(38598, "ISO-8859-8-I");

		addCodePage(1254, "ISO-8859-9");
	}

	static const char trailingBytesForUTF8[256] = {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
	};

	/* returns length of next utf-8 sequence */
	int u8_seqlen(unsigned char c)
	{
		return trailingBytesForUTF8[c] + 1;
	}

	void TextCodecWin::registerEncodingNames(EncodingNameRegistrar registrar) {
		initCharsets();
		registrar("GBK", "GBK");
		registrar("gbk", "GBK");
		registrar("gb2312", "GBK");
		registrar("GB2312", "GBK");
		registrar("GB18030", "GB18030");
		registrar("gb18030", "GB18030");
		registrar("Big5", "Big5");
		registrar("big5", "Big5");
	}

	void TextCodecWin::registerCodecs(TextCodecRegistrar registrar) {
		initCharsets();

		registrar("GBK", create, (void*)936);
		registrar("GB18030", create, (void*)54936);
		registrar("Big5", create, (void*)950);
	}

	PassOwnPtr<TextCodec> TextCodecWin::create(const TextEncoding& encoding, const void* data) {
		int codePage = (int)data;
		const CharsetInfo* charset = getCharset(codePage);
		if (!charset)
			return NULL;

		return adoptPtr(new TextCodecWin(charset));
	}

	bool TextCodecWin::hasValidChar()
	{
		if (m_partialSequence == 0)
			return false;

		if (charset->codePage != CP_UTF8)
		{
			m_partialSequence[m_partialSequenceSize] = 'A';
			m_partialSequence[m_partialSequenceSize + 1] = '\0';
			char* ptr = CharNextExA(charset->codePage, (LPCSTR)m_partialSequence, 0);
			if (ptr > ((char*)m_partialSequence + m_partialSequenceSize))
				return false;

			return true;
		}

		//utf8
		if (u8_seqlen(m_partialSequence[0]) <= m_partialSequenceSize)
			return true;

		return false;
	}

	bool TextCodecWin::toUnicode(unsigned char c, UChar& uc)
	{
		m_partialSequence[m_partialSequenceSize++] = c;
		if (m_partialSequenceSize + 2 >= UCNV_MAX_CHAR_LEN || hasValidChar())
		{
			int ret = MultiByteToWideChar(charset->codePage, 0, (LPSTR)m_partialSequence, m_partialSequenceSize, &uc, 1);
			m_partialSequenceSize = 0;

			return ret == 1 ? true : false;
		}

		return false;
	}

	String TextCodecWin::decode(const char* bytes, size_t length, FlushBehavior flush, bool stopOnError, bool& sawError) {
		StringBuffer<UChar> buffer(m_partialSequenceSize + length);
		const uint8_t* source = reinterpret_cast<const uint8_t*>(bytes);
		const uint8_t* end = source + length;
		UChar* destination = buffer.characters();

		UChar ch;
		while (source < end) {
			if (toUnicode(*source, ch)) {
				*destination = ch;
				destination++;
			}
			source++;
		}
		buffer.shrink(destination - buffer.characters());
		return String::adopt(buffer);
	}

	CString TextCodecWin::encode(const UChar* source, size_t length, UnencodableHandling) {
		int ret = WideCharToMultiByte(charset->codePage, 0, source, length, NULL, 0, NULL, NULL);
		if (ret <= 0)
			return CString();
		Vector<uint8_t> bytes(ret);
		WideCharToMultiByte(charset->codePage, 0, source, length, (LPSTR)bytes.data(), ret, NULL, NULL);
		return CString(reinterpret_cast<char*>(bytes.data()), ret);
	}

	CString TextCodecWin::encode(const LChar* characters, size_t length, UnencodableHandling handling) {
		//bool sawError = false;
		//m_partialSequenceSize = 0;
		//String returnString = decode((const char*)characters, length, DoNotFlush, true, sawError);
		//return encode(returnString.characters16(), returnString.length(), handling);
		return CString(reinterpret_cast<const char*>(characters), length);
	}
}