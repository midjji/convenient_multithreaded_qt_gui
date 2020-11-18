#include <thread>
#include <iostream>
#include <mtgui.h>
#include <QApplication>
#include <QMainWindow>

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
    run_in_gui_thread(new QAppLambda([](){
        QMainWindow* window=new QMainWindow();
        window->show();
    }));

    // non blocking
    run_in_gui_thread(new QAppLambda([](){
        QMainWindow* window=new QMainWindow();
        window->show();
    }));

    // works fine from other threads too...
    std::thread thr=std::thread([](){
        run_in_gui_thread(new QAppLambda([](){
            QMainWindow* window=new QMainWindow();
            window->show();
        }));
    });
    // still non blocking...
    thr.join();
    // We do something else, while the gui is responsive,
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    // or we wait untill the qapp is finished,
    wait_for_qapp_to_finish();

}

void external_app_gui(){
    int i=0;
    QApplication qapp(i,nullptr); // this instance will be used!

    std::thread thr=std::thread([](){
        run_in_gui_thread(new QAppLambda([](){
            QMainWindow* window=new QMainWindow();
            window->show();
        }));
    });
    thr.join();
    qapp.exec();
}


int main(){

    //typical_qt_gui_app();
    thread_independent_qt_gui_app();

    //external_app_gui();
}

