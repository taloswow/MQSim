#ifndef XMLWRITE_H
#define XMLWRITE_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace Utils
{
	class XmlWriter {
	public:
		bool Open(const std::string);
		void Close();
		bool exists(const std::string);
		void WriteOpenTag(const std::string);
		void WriteCloseTag();
		void WriteStartElementTag(const std::string);
		void WriteEndElementTag();
		void WriteAttribute(const std::string);
		void WriteString(const std::string);
		void WriteAttributeString(const std::string attribute_name, const std::string attribute_value);
		void WriteAttributeStringInline(const std::string attribute_name, const std::string attribute_value);
	private:
		std::ofstream outFile;
		int indent;
		int openTags;
		int openElements;
		std::vector<std::string> tempOpenTag;
		std::vector<std::string> tempElementTag;
	};
}

#endif // !XMLWRITE_H
