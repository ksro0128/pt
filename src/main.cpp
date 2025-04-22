#include "include/App.h"

int main()
{
	std::cout << "start" << std::endl;
	try {
		App app;
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...) {
		std::cerr << "Unknown exception" << std::endl;
		return EXIT_FAILURE;
	}
	return 0;
}