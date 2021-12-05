#include <utility>

#include "transport_directory.h"
#include "transport_directory_impl.h"

namespace transport {

TransportDirectory::TransportDirectory(config::Config config)
	: impl_{std::make_unique<TransportDirectoryImpl>(std::move(config))}
{
}

TransportDirectory::~TransportDirectory() = default;

std::optional<info::Bus> TransportDirectory::getBus(
	std::string const &name) const
{
	return impl_->getBus(name);
}

std::optional<info::Stop> TransportDirectory::getStop(
	std::string const &name) const
{
	return impl_->getStop(name);
}

std::optional<info::Route> TransportDirectory::getRoute(
	std::string const &from, std::string const &to) const
{
	return impl_->getRoute(from, to);
}

} // namespace transport
