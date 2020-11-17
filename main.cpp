#include <thread>
#include <QApplication>
#include <QMainWindow>
#include <mtgui.h>

#include <iostream>
using std::endl;using std::cout;
void typical_qt_gui_app(){
    int i=0;
    // will crash if QApplication allready exists
    QApplication qapp(i,nullptr);
    // setup things, but then all interaction must happen through qtevents... annoying
    QMainWindow window;
    window.show();
    // blocking
    qapp.exec();

}

void thread_independent_qt_gui_app(){


    //No need to initialize qt, but it does work
    //int a=0;
    run_in_gui_thread(new RunEventImpl([](){

        QMainWindow* window=new QMainWindow();

        window->show();

    }));

    // non blocking
    run_in_gui_thread(new RunEventImpl([](){
        QMainWindow* window=new QMainWindow();
        window->show();
    }));

    // works fine from other threads too...
    std::thread thr=std::thread([](){
        run_in_gui_thread(new RunEventImpl([](){
            QMainWindow* window=new QMainWindow();
            window->show();
        }));
    });
    // still non blocking...
    thr.join();
    // need to wait to see something.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // or we wait for the qapp to exit...
    wait_for_qapp_to_finish();

}

int main(){

    //typical_qt_gui_app();
    thread_independent_qt_gui_app();


}

