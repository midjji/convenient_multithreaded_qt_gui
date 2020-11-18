# convenient_multithreaded_qt_gui

Qt is great for guis in part because it does not need to run in the main loop, leaving that free for some other main loop hog. 
But gui operations are not reentrant for performance reasons. Which means some additional code is required to write simple functions like: plot(x,y); from any thread. This is that code. It will transparently set up the Qapplication and call exec in a background thread, or work with an existing Qapplication if present. 

To execute any command in the QApplication thread, from any thread, you use the syntax. 


#include <mtgui.h> 

run_in_gui_thread_blocking(new QAppLambda(\[&\](){stuff which will be done by in the QApplication stuff }));

or 

run_in_gui_thread(new QAppLambda(\[\](Args...){stuff which will be done by in the QApplication stuff }, args...));



Remember to only create gui elements in the main loop. 
Otherwize you will get a warning runtime, or if unlucky a segfault. 

This is a rewrite of the older version where inheriting run events is done instead. 
The difference between this solution and alternatives, is that it also automatically initializes and exec qapp for you, while still supporting in place qapps. 
