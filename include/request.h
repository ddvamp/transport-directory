#ifndef REQUEST_H_
#define REQUEST_H_ 1

#include "json.h"
#include "transport_directory.h"

namespace request {

[[nodiscard]] json::Object process(json::Object const &node,
	transport::TransportDirectory const &directory);

[[nodiscard]] json::Array processAll(json::Array const &nodes,
	transport::TransportDirectory const &directory);

} // namespace request

#endif /* REQUEST_H_ */
