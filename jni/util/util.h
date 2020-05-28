#pragma once

std::vector<std::string> splitLineByDelimiter(const std::string& szString, char cDelimiter);

struct splittedText {
	struct {
		std::string szColor = std::string("FFFFFFFF");
		std::string szText = std::string("none");
	} splittedData[128];
	size_t uiCount = 0;
};
splittedText *splitText(std::string szOrigColor, std::string szInput);

uintptr_t FindLibrary(const char* library);
void cp1251_to_utf8(char *out, const char *in, unsigned int len = 0);