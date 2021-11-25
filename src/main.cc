#include <iostream>

#include "description.h"
#include "json.h"
#include "request.h"
#include "transport_directory.h"

int main()
{
	std::ios_base::sync_with_stdio(false);
	std::cin.tie(nullptr);

	auto const document = json::readDocument(std::cin);
	auto const &config = document.getRoot().asObject();

	transport::TransportDirectory directory{
		description::parseConfig(config)};

	auto response = request::processAll(directory,
		config.at("stat_requests").asArray());

	json::writeValue(response, std::cout);

	return 0;
}
