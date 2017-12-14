#include "config.h"
#include "WebMimeRegistryImpl.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include <wtf/text/StringHash.h>
#include <wtf/text/WTFStringUtil.h>
#include <wtf/HashMap.h>

namespace content {
WebMimeRegistryImpl::WebMimeRegistryImpl() 
{
    m_supportedMIMETypes = nullptr;
    m_supportedImageResourceMIMETypes = nullptr;
    m_supportedJavaScriptMIMETypes = nullptr;
    m_supportedNonImageMIMETypes = nullptr;
    m_mimetypeMap = nullptr;
}

WebMimeRegistryImpl::~WebMimeRegistryImpl() 
{
    if (m_supportedMIMETypes)
        delete m_supportedMIMETypes;
    m_supportedMIMETypes = nullptr;

    if (m_supportedImageResourceMIMETypes)
        delete m_supportedImageResourceMIMETypes;
    m_supportedImageResourceMIMETypes = nullptr;

    if (m_supportedJavaScriptMIMETypes)
        delete m_supportedJavaScriptMIMETypes;
    m_supportedJavaScriptMIMETypes = nullptr;

    if (m_supportedNonImageMIMETypes)
        delete m_supportedNonImageMIMETypes;
    m_supportedNonImageMIMETypes = nullptr;

    if (m_mimetypeMap)
        delete m_mimetypeMap;
    m_mimetypeMap = nullptr;
}

blink::WebMimeRegistry::SupportsType WebMimeRegistryImpl::supportsImagePrefixedMIMEType(const blink::WebString& mimeType)
{
    return blink::WebMimeRegistry::IsNotSupported;
}

static WTF::String checkAndEnsureBit8String(const blink::WebString& ext)
{
    if (ext.isNull() || ext.isEmpty())
        return WTF::String();

    WTF::String extension = ext;
    if (!extension.is8Bit()) {
        CString utf8String = extension.utf8();
        extension = WTF::String(utf8String.data(), utf8String.length());
    }

    return extension;
}

// WebMimeRegistry methods:
blink::WebMimeRegistry::SupportsType WebMimeRegistryImpl::supportsMIMEType(const blink::WebString& ext)
{
    WTF::String extension = checkAndEnsureBit8String(ext);
    if (extension.isEmpty())
        return blink::WebMimeRegistry::IsNotSupported;

    if (!m_supportedMIMETypes) {
        m_supportedMIMETypes = new WTF::HashSet<WTF::String>();

        static const char* types[] = {
            "text/plain",
            "text/html",
            "text/xml",
            "application/x-javascript",
            "application/xhtml+xml",
            "image/svg+xml",
            "image/jpeg",
            "image/png",
            "image/tiff",
            "image/ico",
            "image/bmp",
            "image/gif",
            "application/x-shockwave-flash",
        };
        for (size_t i = 0; i < WTF_ARRAY_LENGTH(types); ++i)
            m_supportedMIMETypes->add(types[i]);
    }

    return m_supportedMIMETypes->contains(extension) ? blink::WebMimeRegistry::IsSupported : blink::WebMimeRegistry::IsNotSupported;
}

blink::WebMimeRegistry::SupportsType WebMimeRegistryImpl::supportsImageMIMEType(const blink::WebString& ext)
{
    WTF::String extension = checkAndEnsureBit8String(ext);
    if (extension.isEmpty())
        return blink::WebMimeRegistry::IsNotSupported;

    if (!m_supportedImageResourceMIMETypes) {
        m_supportedImageResourceMIMETypes = new WTF::HashSet<WTF::String>();
        static const char* types[] = {
            //"image/svg+xml",
            "image/jpeg",
            "image/png",
            "image/tiff",
            //"application/x-javascript",
            "image/ico",
            "image/bmp",
            "image/gif",
        };
        for (size_t i = 0; i < WTF_ARRAY_LENGTH(types); ++i)
            m_supportedImageResourceMIMETypes->add(types[i]);
    }

    return m_supportedImageResourceMIMETypes->contains(extension) ? blink::WebMimeRegistry::IsSupported : blink::WebMimeRegistry::IsNotSupported;
}

blink::WebMimeRegistry::SupportsType WebMimeRegistryImpl::supportsJavaScriptMIMEType(const blink::WebString& ext)
{
    WTF::String extension = checkAndEnsureBit8String(ext);
    if (extension.isEmpty())
        return blink::WebMimeRegistry::IsNotSupported;

    if (!m_supportedJavaScriptMIMETypes) {
        m_supportedJavaScriptMIMETypes = new WTF::HashSet<WTF::String>();
        /*
        Mozilla 1.8 and WinIE 7 both accept text/javascript and text/ecmascript.
        Mozilla 1.8 accepts application/javascript, application/ecmascript, and application/x-javascript, but WinIE 7 doesn't.
        WinIE 7 accepts text/javascript1.1 - text/javascript1.3, text/jscript, and text/livescript, but Mozilla 1.8 doesn't.
        Mozilla 1.8 allows leading and trailing whitespace, but WinIE 7 doesn't.
        Mozilla 1.8 and WinIE 7 both accept the empty string, but neither accept a whitespace-only string.
        We want to accept all the values that either of these browsers accept, but not other values.
        */
        static const char* types[] = {
            "text/javascript",
            "text/ecmascript",
            "application/javascript",
            "application/ecmascript",
            "application/x-javascript",
            "text/javascript1.1",
            "text/javascript1.2",
            "text/javascript1.3",
            "text/jscript",
            "text/livescript",
        };
        for (size_t i = 0; i < WTF_ARRAY_LENGTH(types); ++i)
            m_supportedJavaScriptMIMETypes->add(types[i]);
    }

    return m_supportedJavaScriptMIMETypes->contains(extension) ? blink::WebMimeRegistry::IsSupported : blink::WebMimeRegistry::IsNotSupported;
}

blink::WebMimeRegistry::SupportsType WebMimeRegistryImpl::supportsMediaMIMEType(
    const blink::WebString&, const blink::WebString&, const blink::WebString&)
{
    return blink::WebMimeRegistry::IsNotSupported;
}

bool WebMimeRegistryImpl::supportsMediaSourceMIMEType(const blink::WebString&, const blink::WebString&)
{
    return blink::WebMimeRegistry::IsNotSupported;
}

blink::WebMimeRegistry::SupportsType WebMimeRegistryImpl::supportsNonImageMIMEType(const blink::WebString& ext)
{
    WTF::String extension = checkAndEnsureBit8String(ext);
    if (extension.isEmpty())
        return blink::WebMimeRegistry::IsNotSupported;
    
    if (!m_supportedNonImageMIMETypes) {
        m_supportedNonImageMIMETypes = new HashSet<String>();

        static const char* types[] = {
            "text/html",
            "text/xml",
            "text/xsl",
            "text/plain",
            "text/",
            "application/xml",
            "application/xhtml+xml",
            "application/vnd.wap.xhtml+xml",
            "application/rss+xml",
            "application/atom+xml",
            "application/json",
//#if ENABLE(SVG)
            "image/svg+xml",
//#endif
            "application/x-ftp-directory",
            "multipart/x-mixed-replace",
            //"application/x-shockwave-flash",
            // Note: ADDING a new type here will probably render it as HTML. This can
            // result in cross-site scripting.
        };
        //COMPILE_ASSERT(sizeof(types) / sizeof(types[0]) <= 16, "nonimage_mime_types_must_be_less_than_or_equal_to_16");
            
        for (size_t i = 0; i < WTF_ARRAY_LENGTH(types); ++i)
            m_supportedNonImageMIMETypes->add(types[i]);
    }

    return m_supportedNonImageMIMETypes->contains(extension) ? blink::WebMimeRegistry::IsSupported : blink::WebMimeRegistry::IsNotSupported;
}

blink::WebString WebMimeRegistryImpl::mimeTypeForExtension(const blink::WebString& ext)
{
    ASSERT(isMainThread());

    if (ext.isNull() || ext.isEmpty())
        return blink::WebString();

    WTF::String extension = WTF::ensureStringToUTF8String(ext);
    if (!m_mimetypeMap) {
        m_mimetypeMap = new WTF::HashMap<WTF::String, WTF::String>();
        //fill with initial values
        m_mimetypeMap->add("txt", "text/plain");
        m_mimetypeMap->add("pdf", "application/pdf");
        m_mimetypeMap->add("ps", "application/postscript");
        m_mimetypeMap->add("html", "text/html");
        m_mimetypeMap->add("htm", "text/html");
        m_mimetypeMap->add("xml", "text/xml");
        m_mimetypeMap->add("xsl", "text/xsl");
        m_mimetypeMap->add("js", "application/x-javascript");
        m_mimetypeMap->add("xhtml", "application/xhtml+xml");
        m_mimetypeMap->add("rss", "application/rss+xml");
        m_mimetypeMap->add("webarchive", "application/x-webarchive");
        m_mimetypeMap->add("svg", "image/svg+xml");
        m_mimetypeMap->add("svgz", "image/svg+xml");
        m_mimetypeMap->add("jpg", "image/jpeg");
        m_mimetypeMap->add("jpeg", "image/jpeg");
        m_mimetypeMap->add("png", "image/png");
        m_mimetypeMap->add("gif", "image/gif");
        m_mimetypeMap->add("tif", "image/tiff");
        m_mimetypeMap->add("tiff", "image/tiff");
        m_mimetypeMap->add("ico", "image/ico");
        m_mimetypeMap->add("cur", "image/ico");
        m_mimetypeMap->add("bmp", "image/bmp");
        m_mimetypeMap->add("wml", "text/vnd.wap.wml");
        m_mimetypeMap->add("wmlc", "application/vnd.wap.wmlc");
        m_mimetypeMap->add("swf", "application/x-shockwave-flash");
        //
    }
    WTF::String result = m_mimetypeMap->get(extension);
    return result;
}

blink::WebString WebMimeRegistryImpl::wellKnownMimeTypeForExtension(const blink::WebString&)
{
    return blink::WebString();
}

blink::WebString WebMimeRegistryImpl::mimeTypeFromFile(const blink::WebString&)
{
    return blink::WebString();
}

} // namespace content