#ifndef DDV_DESCRIPTION_H_
#define DDV_DESCRIPTION_H_ 1

#include "json.h"
#include "transport_directory_config.h"

namespace description {

[[nodiscard]] transport::config::Bus parseBus(json::Object const &);

[[nodiscard]] transport::config::Stop parseStop(json::Object const &);

[[nodiscard]] transport::config::RoutingSettings
	parseRoutingSettings(json::Object const &);

[[nodiscard]] transport::config::RenderSettings
	parseRenderSettings(json::Object const &);

[[nodiscard]] transport::config::Config parseConfig(json::Object const &);

} // namespace description

#endif /* DDV_DESCRIPTION_H_ */
