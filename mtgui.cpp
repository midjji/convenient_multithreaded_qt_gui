#include <mutex>
#include <thread>
#include <condition_variable>
#include <iostream>
#include <QApplication>

#include <mtgui.h>


namespace {





struct QApplicationManager
{

    // if we dont own, then this gets set when the qapp quits
    std::shared_ptr<std::atomic<bool>> done=std::make_shared<std::atomic<bool>>(false);
    bool we_own_app=true;
    std::thread thr;
    QCoreApplication* app=nullptr;


    ~QApplicationManager()
    {
        // must do quit and delete app in the thread where it was made!
        if(we_own_app ){
            //app->quit();
            if(thr.joinable()) thr.join();
            //delete app;
        }
    }
    static std::shared_ptr<QApplicationManager> create() {
        std::cout<<"create"<<std::endl;
        auto qm=std::make_shared<QApplicationManager>();
        // if an instance already exists, use it.
        // this is for interop with others who does dumb shit like opencv...
        if(QApplication::instance()!=nullptr){
            qm->we_own_app=false;
            qm->app=QCoreApplication::instance();



            qm->app->postEvent(
                        qm->app,
                        new RunEventImpl([](std::shared_ptr<QApplicationManager> qm){
                            QObject::connect(qm->app, &QApplication::aboutToQuit,
                            qm->app, [qm](){ (*qm->done)=true;  });
                        },qm));
            return qm;
        }

        std::atomic<bool> ready=false;
        qm->thr=std::thread([&]()
        {
            int i=0;
            qm->app = new QApplication(i,nullptr); // accessible by instance
            // note that qm is captured by copy here, thus it always exist till the closure is finished i.e. when the QApplication is deleted.
            QObject::connect(qm->app, &QApplication::aboutToQuit, qm->app, [qm](){ (*qm->done)=true; });
            ready=true;
            qm->app->exec();
        });

        while(!ready) std::this_thread::sleep_for(std::chrono::milliseconds(50));

        return qm;
    }

    void wait_for_finished() {

        while(!(*done)) std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
};
std::mutex qapp_mtx;
std::shared_ptr<QApplicationManager> qm=nullptr;
}
std::shared_ptr<QApplicationManager> qapplication_manager(){
    std::unique_lock<std::mutex> ul(qapp_mtx);
    if(qm==nullptr)
        qm=QApplicationManager::create();
    return qm;
}

QCoreApplication* qapplication(){
    return qapplication_manager()->app;
}
void wait_for_qapp_to_finish() {
    qapplication_manager()->wait_for_finished();
}

void run_in_gui_thread(RunEvent* re){
    // note, execution order matters!
    auto qm=qapplication();
    // thread safe!
    qm->postEvent(qm,re);
    // will return before event is executed
}

namespace  {
struct Blocker:public RunEvent{
    RunEvent* re;
    std::shared_ptr<std::atomic<bool>> done;
    Blocker(RunEvent* re, std::shared_ptr<std::atomic<bool>> done):re(re),done(done){}
    void run(){}
    ~Blocker(){
        delete re;
        (*done)=true;
    }
};
}
void run_in_gui_thread_blocking(RunEvent* re){
    std::shared_ptr<std::atomic<bool>> done=std::make_shared<std::atomic<bool>>(false);
    // can I create a QObject before QApplication?
    run_in_gui_thread(new Blocker(re,done));
    // if latency is an issue, consider replacing with mutexes, cvs, and wait....
    // but it probably isnt, you dont do this alot...
    while(!done) std::this_thread::sleep_for(std::chrono::milliseconds(50));
}
