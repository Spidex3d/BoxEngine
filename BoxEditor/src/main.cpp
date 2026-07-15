#include <BoxEngine.h>
#include <BoxWindow.h>
#include <miniBoxLog.h>


int main() {

	BOX_LOG_INFO("Hello, from the BoxEditor!");
	BoxEngine boxE;

	WindowConfig cfg;
	cfg.width = 800;
	cfg.height = 600;
	cfg.title = "Box Editor Test Window";

	// Initialize the window with config 
	


	boxE.testFunction();

	return 0;
}