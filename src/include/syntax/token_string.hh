#pragma once
#include "tokens.hh"

#include <string>

namespace syntax
{
#define ARRAY_SZ(a) (sizeof(a)/sizeof(a[0]))

std::string g_token_name = "EOFnameliteralopop=opop=:=<-*([{)]},;:....breakcasechanconstcontinuedefaultdeferelsefallthroughforfuncgogotoifimportinterfacemappackagerangereturnselectstructswitchtypevar";
uint8_t g_token_index[] = {0, 3, 7, 14, 16, 19, 23, 24, 26, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 42, 47, 51, 55, 60, 68, 75, 80, 84, 95, 98, 102, 104, 108, 110, 116, 125, 128, 135, 140, 146, 152, 158, 164, 168, 171, 171};

inline std::string TokenString(token i) {
	i -= 1;
	if (i >= token(ARRAY_SZ(g_token_index)-1)) {
		return "token(" + to_string(i) + ")";
	}
	return g_token_name.substr(g_token_index[i], g_token_index[i+1] - g_token_index[i]);
}

} // namespace syntax
