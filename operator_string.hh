#pragma once
#include "tokens.hh"

#include <string>
namespace syntax
{
const std::string g_operator_name = ":!<-~||&&==!=<<=>>=+-|^*/%&&^<<>>";

const uint8_t g_operator_index[] = {0, 1, 2, 4, 5, 7, 9, 11, 13, 14, 16, 17, 19, 20, 21, 22, 23, 24, 25, 26, 27, 29, 31, 33};

inline std::string OperatorString (Operator i) {
	i -= 1;
	if (i >= Operator(ARRAY_SZ(g_operator_index)-1)) {
		return "Operator(" + to_string(i) + ")";
	}
	return g_operator_name.substr(g_operator_index[i], g_operator_index[i+1] - g_operator_index[i]);
}
} // namespace syntax
