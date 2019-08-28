#include <json/writer.h>
#include <utility>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <iomanip>

#if _MSC_VER >= 1400 // VC++ 8.0
#pragma warning( disable : 4996 )   // disable warning about strdup being deprecated.
#endif

#define FAILURE (-1)
#define SUCCESS (0)

#define MB_FAILURE(pos, advance) do { \
    *cursor = pos + (advance); \
    *status = FAILURE; \
    return 0; \
} while (0)
#define CHECK_LEN(pos, chars_need) ((str_len - (pos)) >= (chars_need))

#define utf8_trail(c) ((c) >= 0x80 && (c) <= 0xBF)
#define utf8_lead(c)  ((c) < 0x80 || ((c) >= 0xC2 && (c) <= 0xF4))

static const char digits[] = "0123456789abcdef";

namespace Json
{

static inline unsigned int php_next_utf8_char(const unsigned char *str, size_t str_len, size_t *cursor, int *status)
{
    size_t pos = *cursor;
    unsigned int this_char = 0;

    *status = SUCCESS;
    assert(pos <= str_len);

    if (!CHECK_LEN(pos, 1))
        MB_FAILURE(pos, 1);

    /* We'll follow strategy 2. from section 3.6.1 of UTR #36:
     * "In a reported illegal byte sequence, do not include any
     *  non-initial byte that encodes a valid character or is a leading
     *  byte for a valid sequence." */
    unsigned char c;
    c = str[pos];
    if (c < 0x80) {
        this_char = c;
        pos++;
    } else if (c < 0xc2) {
        MB_FAILURE(pos, 1);
    } else if (c < 0xe0) {
        if (!CHECK_LEN(pos, 2))
            MB_FAILURE(pos, 1);

        if (!utf8_trail(str[pos + 1])) {
            MB_FAILURE(pos, utf8_lead(str[pos + 1]) ? 1 : 2);
        }
        this_char = ((c & 0x1f) << 6) | (str[pos + 1] & 0x3f);
        if (this_char < 0x80) { /* non-shortest form */
            MB_FAILURE(pos, 2);
        }
        pos += 2;
    } else if (c < 0xf0) {
        size_t avail = str_len - pos;

        if (avail < 3 ||
                !utf8_trail(str[pos + 1]) || !utf8_trail(str[pos + 2])) {
            if (avail < 2 || utf8_lead(str[pos + 1]))
                MB_FAILURE(pos, 1);
            else if (avail < 3 || utf8_lead(str[pos + 2]))
                MB_FAILURE(pos, 2);
            else
                MB_FAILURE(pos, 3);
        }

        this_char = ((c & 0x0f) << 12) | ((str[pos + 1] & 0x3f) << 6) | (str[pos + 2] & 0x3f);
        if (this_char < 0x800) { /* non-shortest form */
            MB_FAILURE(pos, 3);
        } else if (this_char >= 0xd800 && this_char <= 0xdfff) { /* surrogate */
            MB_FAILURE(pos, 3);
        }
        pos += 3;
    } else if (c < 0xf5) {
        size_t avail = str_len - pos;

        if (avail < 4 ||
                !utf8_trail(str[pos + 1]) || !utf8_trail(str[pos + 2]) ||
                !utf8_trail(str[pos + 3])) {
            if (avail < 2 || utf8_lead(str[pos + 1]))
                MB_FAILURE(pos, 1);
            else if (avail < 3 || utf8_lead(str[pos + 2]))
                MB_FAILURE(pos, 2);
            else if (avail < 4 || utf8_lead(str[pos + 3]))
                MB_FAILURE(pos, 3);
            else
                MB_FAILURE(pos, 4);
        }

        this_char = ((c & 0x07) << 18) | ((str[pos + 1] & 0x3f) << 12) | ((str[pos + 2] & 0x3f) << 6) | (str[pos + 3] & 0x3f);
        if (this_char < 0x10000 || this_char > 0x10FFFF) { /* non-shortest form or outside range */
            MB_FAILURE(pos, 4);
        }
        pos += 4;
    } else {
        MB_FAILURE(pos, 1);
    }

    *cursor = pos;
    return this_char;
}

static int json_utf8_to_utf16(unsigned short *utf16, const char* utf8, int len)
{
	size_t pos = 0, us;
	int j, status;

	if (utf16) {
		/* really convert the utf8 string */
		for (j=0 ; pos < (size_t)len ; j++) {
			us = php_next_utf8_char((const unsigned char *)utf8, len, &pos, &status);
			if (status != SUCCESS) {
				return -1;
			}
			/* From http://en.wikipedia.org/wiki/UTF16 */
			if (us >= 0x10000) {
				us -= 0x10000;
				utf16[j++] = (unsigned short)((us >> 10) | 0xd800);
				utf16[j] = (unsigned short)((us & 0x3ff) | 0xdc00);
			} else {
				utf16[j] = (unsigned short)us;
			}
		}
	} else {
		/* Only check if utf8 string is valid, and compute utf16 lenght */
		for (j=0 ; pos < (size_t)len ; j++) {
			us = php_next_utf8_char((const unsigned char *)utf8, len, &pos, &status);
			if (status != SUCCESS) {
				return -1;
			}
			if (us >= 0x10000) {
				j++;
			}
		}
	}
	return j;
}


static bool isControlCharacter(char ch)
{
    return ch > 0 && ch <= 0x1F;
}

static bool containsControlCharacter(const char *str)
{
    while (*str) {
        if (isControlCharacter(*(str++))) {
            return true;
        }
    }
    return false;
}

static void uintToString(unsigned int value,
                         char *&current)
{
    *--current = 0;
    do {
        *--current = (value % 10) + '0';
        value /= 10;
    } while (value != 0);
}

std::string valueToString(Int value)
{
    char buffer[32];
    char *current = buffer + sizeof(buffer);
    bool isNegative = value < 0;
    if (isNegative) {
        value = -value;
    }
    uintToString(UInt(value), current);
    if (isNegative) {
        *--current = '-';
    }
    assert(current >= buffer);
    return current;
}


std::string valueToString(UInt value)
{
    char buffer[32];
    char *current = buffer + sizeof(buffer);
    uintToString(value, current);
    assert(current >= buffer);
    return current;
}

std::string valueToString(double value)
{
    char buffer[32];
#if defined(_MSC_VER) && defined(__STDC_SECURE_LIB__) // Use secure version with visual studio 2005 to avoid warning.
    sprintf_s(buffer, sizeof(buffer), "%#.16g", value);
#else
    snprintf(buffer, sizeof(buffer) - 1, "%#.16g", value);
#endif
    char *ch = buffer + strlen(buffer) - 1;
    if (*ch != '0') {
        return buffer;    // nothing to truncate, so save time
    }
    while (ch > buffer && *ch == '0') {
        --ch;
    }
    char *last_nonzero = ch;
    while (ch >= buffer) {
        switch (*ch) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                --ch;
                continue;
            case '.':
                // Truncate zeroes to save bytes in output, but keep one.
                *(last_nonzero + 2) = '\0';
                return buffer;
            default:
                return buffer;
        }
    }
    return buffer;
}


std::string valueToString(bool value)
{
    return value ? "true" : "false";
}

class safe_ptr
{
public:
    char* buf;
    int len;

    safe_ptr(int size)
    {
        buf = new char[size];
        len = size;
    }

    ~safe_ptr()
    {
        if (NULL != buf) {
            delete []buf;
        }
    }
};

std::string valueToUnicodeString(const char *value, int size)
{
/*
    // Not sure how to handle unicode...
    if (strpbrk(value, "\"\\\b\f\n\r\t") == NULL && !containsControlCharacter(value)) {
        return std::string("\"") + value + "\"";
    }
*/
    int pos = 0, ulen = 0;
	unsigned short us;
    safe_ptr safep(size*2);
    unsigned short* utf16 = (unsigned short*)safep.buf;
	ulen = json_utf8_to_utf16(utf16, value, size);

    // We have to walk value and escape any special characters.
    // Appending to std::string is not efficient, but this should be rare.
    // (Note: forward slashes are *not* rare, but I am not escaping them.)
    unsigned maxsize = ulen + 3; // allescaped+quotes+NULL
    std::string result;
    result.reserve(maxsize); // to avoid lots of mallocs
    result += "\"";


    while (pos < ulen) {
        us = utf16[pos++];

        switch (us) {
            //case '\"':
            //    result += "\\\"";
            //    break;
            case '\\':
                result += "\\\\";
                break;
            case '\b':
                result += "\\b";
                break;
            case '\f':
                result += "\\f";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            //html special chars ,except the unicode   
            case '<': 
                result += "\\u003C"; 
                break;
            case '\'': 
                result += "\\u0027"; 
                break;
			case '\"': 
                result += "\\u0022"; 
                break;

                //case '/':
                // Even though \/ is considered a legal escape in JSON, a bare
                // slash is also legal, so I see no reason to escape it.
                // (I hope I am not misunderstanding something.
                // blep notes: actually escaping \/ may be useful in javascript to avoid </
                // sequence.
                // Should add a flag to allow this compatibility mode and prevent this
                // sequence from occurring.
            default:
                if (us >= ' ' && ((us & 127) == us)) {
                    result += (unsigned char)us;
                } else {
                    std::ostringstream oss;
                    oss << "\\u" << digits[(us & 0xf000) >> 12] << digits[(us & 0xf00) >> 8]
                        << digits[(us & 0xf0) >> 4] << digits[(us & 0xf)];
                    result += oss.str();
                }

                break;
        }
    }

    result += "\"";
    return result;
}

std::string valueToQuotedString(const char *value)
{
    // Not sure how to handle unicode...
    if (strpbrk(value, "\"\\\b\f\n\r\t") == NULL && !containsControlCharacter(value)) {
        return std::string("\"") + value + "\"";
    }
    // We have to walk value and escape any special characters.
    // Appending to std::string is not efficient, but this should be rare.
    // (Note: forward slashes are *not* rare, but I am not escaping them.)
    unsigned maxsize = strlen(value) * 2 + 3; // allescaped+quotes+NULL
    std::string result;
    result.reserve(maxsize); // to avoid lots of mallocs
    result += "\"";
    for (const char *c = value; *c != 0; ++c) {
        switch (*c) {
            case '\"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\b':
                result += "\\b";
                break;
            case '\f':
                result += "\\f";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
                //case '/':
                // Even though \/ is considered a legal escape in JSON, a bare
                // slash is also legal, so I see no reason to escape it.
                // (I hope I am not misunderstanding something.
                // blep notes: actually escaping \/ may be useful in javascript to avoid </
                // sequence.
                // Should add a flag to allow this compatibility mode and prevent this
                // sequence from occurring.
            default:
                if (isControlCharacter(*c)) {
                    std::ostringstream oss;
                    oss << "\\u" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << static_cast<int>(*c);
                    result += oss.str();
                } else {
                    result += *c;
                }
                break;
        }
    }
    result += "\"";
    return result;
}

// Class Writer
// //////////////////////////////////////////////////////////////////
Writer::~Writer()
{
}


// Class FastWriter
// //////////////////////////////////////////////////////////////////

FastWriter::FastWriter()
    : yamlCompatiblityEnabled_(false)
{
}


void
FastWriter::enableYAMLCompatibility()
{
    yamlCompatiblityEnabled_ = true;
}


std::string
FastWriter::write(const Value &root)
{
    document_ = "";
    writeValue(root);
    document_ += "\n";
    return document_;
}


void
FastWriter::writeValue(const Value &value)
{
    switch (value.type()) {
        case nullValue:
            document_ += "null";
            break;
        case intValue:
            document_ += valueToString(value.asInt());
            break;
        case uintValue:
            document_ += valueToString(value.asUInt());
            break;
        case realValue:
            document_ += valueToString(value.asDouble());
            break;
        case stringValue:
            document_ += valueToQuotedString(value.asCString());
            break;
        case booleanValue:
            document_ += valueToString(value.asBool());
            break;
        case arrayValue: {
                document_ += "[";
                int size = value.size();
                for (int index = 0; index < size; ++index) {
                    if (index > 0) {
                        document_ += ",";
                    }
                    writeValue(value[index]);
                }
                document_ += "]";
            }
            break;
        case objectValue: {
                Value::Members members(value.getMemberNames());
                document_ += "{";
                for (Value::Members::iterator it = members.begin();
                        it != members.end();
                        ++it) {
                    const std::string &name = *it;
                    if (it != members.begin()) {
                        document_ += ",";
                    }
                    document_ += valueToQuotedString(name.c_str());
                    document_ += yamlCompatiblityEnabled_ ? ": "
                                 : ":";
                    writeValue(value[name]);
                }
                document_ += "}";
            }
            break;
        case unicodeValue: {
            std::string strValue = value.asCString();
            document_ += valueToUnicodeString(strValue.c_str(), strValue.size());
        }
        break;
    }
}


// Class StyledWriter
// //////////////////////////////////////////////////////////////////

StyledWriter::StyledWriter()
    : rightMargin_(74)
    , indentSize_(3)
{
}


std::string
StyledWriter::write(const Value &root)
{
    document_ = "";
    addChildValues_ = false;
    indentString_ = "";
    writeCommentBeforeValue(root);
    writeValue(root);
    writeCommentAfterValueOnSameLine(root);
    document_ += "\n";
    return document_;
}


void
StyledWriter::writeValue(const Value &value)
{
    switch (value.type()) {
        case nullValue:
            pushValue("null");
            break;
        case intValue:
            pushValue(valueToString(value.asInt()));
            break;
        case uintValue:
            pushValue(valueToString(value.asUInt()));
            break;
        case realValue:
            pushValue(valueToString(value.asDouble()));
            break;
        case unicodeValue:
        case stringValue:
            pushValue(valueToQuotedString(value.asCString()));
            break;
        case booleanValue:
            pushValue(valueToString(value.asBool()));
            break;
        case arrayValue:
            writeArrayValue(value);
            break;
        case objectValue: {
                Value::Members members(value.getMemberNames());
                if (members.empty()) {
                    pushValue("{}");
                } else {
                    writeWithIndent("{");
                    indent();
                    Value::Members::iterator it = members.begin();
                    while (true) {
                        const std::string &name = *it;
                        const Value &childValue = value[name];
                        writeCommentBeforeValue(childValue);
                        writeWithIndent(valueToQuotedString(name.c_str()));
                        document_ += " : ";
                        writeValue(childValue);
                        if (++it == members.end()) {
                            writeCommentAfterValueOnSameLine(childValue);
                            break;
                        }
                        document_ += ",";
                        writeCommentAfterValueOnSameLine(childValue);
                    }
                    unindent();
                    writeWithIndent("}");
                }
            }
            break;
    }
}


void
StyledWriter::writeArrayValue(const Value &value)
{
    unsigned size = value.size();
    if (size == 0) {
        pushValue("[]");
    } else {
        bool isArrayMultiLine = isMultineArray(value);
        if (isArrayMultiLine) {
            writeWithIndent("[");
            indent();
            bool hasChildValue = !childValues_.empty();
            unsigned index = 0;
            while (true) {
                const Value &childValue = value[index];
                writeCommentBeforeValue(childValue);
                if (hasChildValue) {
                    writeWithIndent(childValues_[index]);
                } else {
                    writeIndent();
                    writeValue(childValue);
                }
                if (++index == size) {
                    writeCommentAfterValueOnSameLine(childValue);
                    break;
                }
                document_ += ",";
                writeCommentAfterValueOnSameLine(childValue);
            }
            unindent();
            writeWithIndent("]");
        } else { // output on a single line
            assert(childValues_.size() == size);
            document_ += "[ ";
            for (unsigned index = 0; index < size; ++index) {
                if (index > 0) {
                    document_ += ", ";
                }
                document_ += childValues_[index];
            }
            document_ += " ]";
        }
    }
}


bool
StyledWriter::isMultineArray(const Value &value)
{
    int size = value.size();
    bool isMultiLine = size * 3 >= rightMargin_ ;
    childValues_.clear();
    for (int index = 0; index < size  &&  !isMultiLine; ++index) {
        const Value &childValue = value[index];
        isMultiLine = isMultiLine  ||
                      ((childValue.isArray()  ||  childValue.isObject())  &&
                       childValue.size() > 0);
    }
    if (!isMultiLine) { // check if line length > max line length
        childValues_.reserve(size);
        addChildValues_ = true;
        int lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
        for (int index = 0; index < size  &&  !isMultiLine; ++index) {
            writeValue(value[index]);
            lineLength += int(childValues_[index].length());
            isMultiLine = isMultiLine  &&  hasCommentForValue(value[index]);
        }
        addChildValues_ = false;
        isMultiLine = isMultiLine  ||  lineLength >= rightMargin_;
    }
    return isMultiLine;
}


void
StyledWriter::pushValue(const std::string &value)
{
    if (addChildValues_) {
        childValues_.push_back(value);
    } else {
        document_ += value;
    }
}


void
StyledWriter::writeIndent()
{
    if (!document_.empty()) {
        char last = document_[document_.length() - 1];
        if (last == ' ') {     // already indented
            return;
        }
        if (last != '\n') {    // Comments may add new-line
            document_ += '\n';
        }
    }
    document_ += indentString_;
}


void
StyledWriter::writeWithIndent(const std::string &value)
{
    writeIndent();
    document_ += value;
}


void
StyledWriter::indent()
{
    indentString_ += std::string(indentSize_, ' ');
}


void
StyledWriter::unindent()
{
    assert(int(indentString_.size()) >= indentSize_);
    indentString_.resize(indentString_.size() - indentSize_);
}


void
StyledWriter::writeCommentBeforeValue(const Value &root)
{
    if (!root.hasComment(commentBefore)) {
        return;
    }
    document_ += normalizeEOL(root.getComment(commentBefore));
    document_ += "\n";
}


void
StyledWriter::writeCommentAfterValueOnSameLine(const Value &root)
{
    if (root.hasComment(commentAfterOnSameLine)) {
        document_ += " " + normalizeEOL(root.getComment(commentAfterOnSameLine));
    }

    if (root.hasComment(commentAfter)) {
        document_ += "\n";
        document_ += normalizeEOL(root.getComment(commentAfter));
        document_ += "\n";
    }
}


bool
StyledWriter::hasCommentForValue(const Value &value)
{
    return value.hasComment(commentBefore)
           ||  value.hasComment(commentAfterOnSameLine)
           ||  value.hasComment(commentAfter);
}


std::string
StyledWriter::normalizeEOL(const std::string &text)
{
    std::string normalized;
    normalized.reserve(text.length());
    const char *begin = text.c_str();
    const char *end = begin + text.length();
    const char *current = begin;
    while (current != end) {
        char c = *current++;
        if (c == '\r') { // mac or dos EOL
            if (*current == '\n') { // convert dos EOL
                ++current;
            }
            normalized += '\n';
        } else { // handle unix EOL & other char
            normalized += c;
        }
    }
    return normalized;
}


// Class StyledStreamWriter
// //////////////////////////////////////////////////////////////////

StyledStreamWriter::StyledStreamWriter(std::string indentation)
    : document_(NULL)
    , rightMargin_(74)
    , indentation_(indentation)
{
}


void
StyledStreamWriter::write(std::ostream &out, const Value &root)
{
    document_ = &out;
    addChildValues_ = false;
    indentString_ = "";
    writeCommentBeforeValue(root);
    writeValue(root);
    writeCommentAfterValueOnSameLine(root);
    *document_ << "\n";
    document_ = NULL; // Forget the stream, for safety.
}


void
StyledStreamWriter::writeValue(const Value &value)
{
    switch (value.type()) {
        case nullValue:
            pushValue("null");
            break;
        case intValue:
            pushValue(valueToString(value.asInt()));
            break;
        case uintValue:
            pushValue(valueToString(value.asUInt()));
            break;
        case realValue:
            pushValue(valueToString(value.asDouble()));
            break;
        case unicodeValue:
        case stringValue:
            pushValue(valueToQuotedString(value.asCString()));
            break;
        case booleanValue:
            pushValue(valueToString(value.asBool()));
            break;
        case arrayValue:
            writeArrayValue(value);
            break;
        case objectValue: {
                Value::Members members(value.getMemberNames());
                if (members.empty()) {
                    pushValue("{}");
                } else {
                    writeWithIndent("{");
                    indent();
                    Value::Members::iterator it = members.begin();
                    while (true) {
                        const std::string &name = *it;
                        const Value &childValue = value[name];
                        writeCommentBeforeValue(childValue);
                        writeWithIndent(valueToQuotedString(name.c_str()));
                        *document_ << " : ";
                        writeValue(childValue);
                        if (++it == members.end()) {
                            writeCommentAfterValueOnSameLine(childValue);
                            break;
                        }
                        *document_ << ",";
                        writeCommentAfterValueOnSameLine(childValue);
                    }
                    unindent();
                    writeWithIndent("}");
                }
            }
            break;
    }
}


void
StyledStreamWriter::writeArrayValue(const Value &value)
{
    unsigned size = value.size();
    if (size == 0) {
        pushValue("[]");
    } else {
        bool isArrayMultiLine = isMultineArray(value);
        if (isArrayMultiLine) {
            writeWithIndent("[");
            indent();
            bool hasChildValue = !childValues_.empty();
            unsigned index = 0;
            while (true) {
                const Value &childValue = value[index];
                writeCommentBeforeValue(childValue);
                if (hasChildValue) {
                    writeWithIndent(childValues_[index]);
                } else {
                    writeIndent();
                    writeValue(childValue);
                }
                if (++index == size) {
                    writeCommentAfterValueOnSameLine(childValue);
                    break;
                }
                *document_ << ",";
                writeCommentAfterValueOnSameLine(childValue);
            }
            unindent();
            writeWithIndent("]");
        } else { // output on a single line
            assert(childValues_.size() == size);
            *document_ << "[ ";
            for (unsigned index = 0; index < size; ++index) {
                if (index > 0) {
                    *document_ << ", ";
                }
                *document_ << childValues_[index];
            }
            *document_ << " ]";
        }
    }
}


bool
StyledStreamWriter::isMultineArray(const Value &value)
{
    int size = value.size();
    bool isMultiLine = size * 3 >= rightMargin_ ;
    childValues_.clear();
    for (int index = 0; index < size  &&  !isMultiLine; ++index) {
        const Value &childValue = value[index];
        isMultiLine = isMultiLine  ||
                      ((childValue.isArray()  ||  childValue.isObject())  &&
                       childValue.size() > 0);
    }
    if (!isMultiLine) { // check if line length > max line length
        childValues_.reserve(size);
        addChildValues_ = true;
        int lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
        for (int index = 0; index < size  &&  !isMultiLine; ++index) {
            writeValue(value[index]);
            lineLength += int(childValues_[index].length());
            isMultiLine = isMultiLine  &&  hasCommentForValue(value[index]);
        }
        addChildValues_ = false;
        isMultiLine = isMultiLine  ||  lineLength >= rightMargin_;
    }
    return isMultiLine;
}


void
StyledStreamWriter::pushValue(const std::string &value)
{
    if (addChildValues_) {
        childValues_.push_back(value);
    } else {
        *document_ << value;
    }
}


void
StyledStreamWriter::writeIndent()
{
    /*
      Some comments in this method would have been nice. ;-)

     if ( !document_.empty() )
     {
        char last = document_[document_.length()-1];
        if ( last == ' ' )     // already indented
           return;
        if ( last != '\n' )    // Comments may add new-line
           *document_ << '\n';
     }
    */
    *document_ << '\n' << indentString_;
}


void
StyledStreamWriter::writeWithIndent(const std::string &value)
{
    writeIndent();
    *document_ << value;
}


void
StyledStreamWriter::indent()
{
    indentString_ += indentation_;
}


void
StyledStreamWriter::unindent()
{
    assert(indentString_.size() >= indentation_.size());
    indentString_.resize(indentString_.size() - indentation_.size());
}


void
StyledStreamWriter::writeCommentBeforeValue(const Value &root)
{
    if (!root.hasComment(commentBefore)) {
        return;
    }
    *document_ << normalizeEOL(root.getComment(commentBefore));
    *document_ << "\n";
}


void
StyledStreamWriter::writeCommentAfterValueOnSameLine(const Value &root)
{
    if (root.hasComment(commentAfterOnSameLine)) {
        *document_ << " " + normalizeEOL(root.getComment(commentAfterOnSameLine));
    }

    if (root.hasComment(commentAfter)) {
        *document_ << "\n";
        *document_ << normalizeEOL(root.getComment(commentAfter));
        *document_ << "\n";
    }
}


bool
StyledStreamWriter::hasCommentForValue(const Value &value)
{
    return value.hasComment(commentBefore)
           ||  value.hasComment(commentAfterOnSameLine)
           ||  value.hasComment(commentAfter);
}


std::string
StyledStreamWriter::normalizeEOL(const std::string &text)
{
    std::string normalized;
    normalized.reserve(text.length());
    const char *begin = text.c_str();
    const char *end = begin + text.length();
    const char *current = begin;
    while (current != end) {
        char c = *current++;
        if (c == '\r') { // mac or dos EOL
            if (*current == '\n') { // convert dos EOL
                ++current;
            }
            normalized += '\n';
        } else { // handle unix EOL & other char
            normalized += c;
        }
    }
    return normalized;
}


std::ostream &operator<<(std::ostream &sout, const Value &root)
{
    Json::StyledStreamWriter writer;
    writer.write(sout, root);
    return sout;
}


} // namespace Json
