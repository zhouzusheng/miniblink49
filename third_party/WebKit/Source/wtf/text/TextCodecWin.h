#ifndef TextCodecWin_h
#define TextCodecWin_h

#include <unicode/utypes.h>
#include "wtf/text/TextCodec.h"
#include "wtf/text/TextEncoding.h"

#define MAX_NAME_LEN 128
#define MAX_CHARSET_COUNT 256
#define MAX_ALIAS_COUNT 16
#define UCNV_MAX_CHAR_LEN 10

namespace WTF {

	class TextCodecInput;
	struct CharsetInfo
	{
		char name[MAX_NAME_LEN];
		unsigned int codePage;

		char aliases[MAX_ALIAS_COUNT][MAX_NAME_LEN];
		unsigned int aliasCount;
	};

	class TextCodecWin final : public TextCodec {
	public:
		static void registerEncodingNames(EncodingNameRegistrar);
		static void registerCodecs(TextCodecRegistrar);

		String decode(const char*, size_t length, FlushBehavior, bool stopOnError, bool& sawError) override;
		CString encode(const UChar*, size_t length, UnencodableHandling) override;
		CString encode(const LChar*, size_t length, UnencodableHandling) override;

	private:
		TextCodecWin(const CharsetInfo* chs) : m_partialSequenceSize(0), charset(chs)
		{
		}
		static PassOwnPtr<TextCodec> create(const TextEncoding&, const void*);

		bool hasValidChar();
		bool toUnicode(unsigned char c, UChar& uc);

		int m_partialSequenceSize;
		uint8_t m_partialSequence[UCNV_MAX_CHAR_LEN];
		const CharsetInfo* charset;
	};

}

#endif