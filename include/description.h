#ifndef DESCRIPTION_H_
#define DESCRIPTION_H_ 1

#include "json.h"
#include "transport_directory_setup.h"

namespace description {

[[nodiscard]] transport::config::Bus parseBus(json::Object const &node);

[[nodiscard]] transport::config::Stop parseStop(json::Object const &node);

[[nodiscard]] transport::config::Config parseConfig(json::Object const &node);

} // namespace description

#endif /* DESCRIPTION_H_ */
