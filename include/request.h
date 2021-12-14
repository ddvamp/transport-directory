#ifndef DDV_REQUEST_H_
#define DDV_REQUEST_H_ 1

#include "json.h"
#include "transport_directory.h"

namespace request {

[[nodiscard]] json::Object process(json::Object const &request,
	transport::TransportDirectory const &database);

[[nodiscard]] json::Array processAll(json::Array const &requests,
	transport::TransportDirectory const &database);

} // namespace request

#endif /* DDV_REQUEST_H_ */
