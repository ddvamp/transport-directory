#ifndef TRANSPORT_DIRECTORY_H_
#define TRANSPORT_DIRECTORY_H_ 1

#include <memory>
#include <optional>
#include <string>

#include "transport_directory_info.h"
#include "transport_directory_setup.h"

namespace transport {

class TransportDirectory {
public:
	TransportDirectory(config::Config config);
	~TransportDirectory();

	[[nodiscard]] std::optional<info::Bus> getBus(
		std::string const &name) const;
	[[nodiscard]] std::optional<info::Stop> getStop(
		std::string const &name) const;
	[[nodiscard]] std::optional<info::Route> getRoute(
		std::string const &from, std::string const &to) const;

private:
	std::unique_ptr<class TransportDirectoryImpl> impl_;
};

} // namespace transport

#endif /* TRANSPORT_DIRECTORY_H_ */
