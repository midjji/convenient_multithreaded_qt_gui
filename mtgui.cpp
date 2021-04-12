#include <mtgui.h>
#include <mutex>
#include <thread>
#include <QEvent>
#include <QApplication>
#include <iostream>

namespace {
struct AnyQAppLambdaEvent:public QEvent{
    AnyQAppLambda* al=nullptr;
    // the event id is just a random number between 1000 and ushort max, and does not matter at all...
    AnyQAppLambdaEvent(AnyQAppLambda* al):QEvent(QEvent::Type(48301)),al(al){}
    virtual ~AnyQAppLambdaEvent(){
        if(al!=nullptr)
            al->run();
        delete al;
    }
};
struct BlockingEvent:public AnyQAppLambdaEvent{

    std::atomic<bool>* done;
    BlockingEvent(AnyQAppLambda* al, std::atomic<bool>* done):
        AnyQAppLambdaEvent(al),done(done){}
    ~BlockingEvent(){

        // order matters, ul must unlock after this!
        if(this->al!=nullptr)
            this->al->run();
        delete al;
        al=nullptr;
        // lowest level destructor first
        // then second lowest, osv
        done->store(true);

    }

};

struct QApplicationManager
{

    // if we dont own, then this gets set when the QApp quits
    std::shared_ptr<std::atomic<bool>> done=std::make_shared<std::atomic<bool>>(false);
    bool we_own_app=true;
    std::thread thr;
    QCoreApplication* app=nullptr;


    ~QApplicationManager()
    {
        // this will intentionally wait for the QApp to finish...
        if(we_own_app ){
            quit();
            if(thr.joinable()) thr.join();
        }
    }
    static std::shared_ptr<QApplicationManager> create(int argc, char** argv) {

        auto qm=std::make_shared<QApplicationManager>();
        // if an instance already exists, use it.
        // this is for interop with others who does dumb shit like opencv...
        if(QApplication::instance()!=nullptr){
            qm->we_own_app=false;
            qm->app=QCoreApplication::instance();
            std::cout<<"warning, the plotter is not managing the qapp instance, did you remember to start it? preferably just dont create a qapp on your own at all"<<std::endl;



            qm->app->postEvent(
                        qm->app,
                        new AnyQAppLambdaEvent(new QAppLambda(
                                                   [qm](){
                QObject::connect(qm->app, &QApplication::aboutToQuit,
                                 qm->app, [qm](){ (*qm->done)=true;  });
            })));
            return qm;
        }

        std::atomic<bool> ready=false;
        qm->thr=std::thread([&]()
        {
            qm->app = new class QApplication(argc,argv); // accessible by instance
            // note that qm is captured by copy here, thus it always exist till the closure is finished i.e. when the QApplication is deleted.
            QObject::connect(qm->app, &QApplication::aboutToQuit, qm->app, [qm](){ (*qm->done)=true; });
            ready=true;
            qm->app->exec();
        });

        while(!ready) std::this_thread::sleep_for(std::chrono::milliseconds(50));
std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return qm;
    }

    void wait_for_finished() {

        while(!(*done)) std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

};
std::mutex QApp_mtx;
std::shared_ptr<QApplicationManager> qm=nullptr;

std::shared_ptr<QApplicationManager> qapplication_manager(int argc=0, char** argv=nullptr){
    std::unique_lock<std::mutex> ul(QApp_mtx);
    if(qm==nullptr)
        qm=QApplicationManager::create(argc,argv);
    //if(!qm->we_own_app)        std::cout<<"warning, the plotter is not managing the qapp instance, did you remember to start it? "<<std::endl;

    return qm;
}
}

QCoreApplication* qapplication(int argc, char** argv){
    return qapplication_manager(argc,argv)->app;
}
void wait_for_qapp_to_finish() {

    qapplication_manager()->wait_for_finished();
}


void run_in_gui_thread(AnyQAppLambda* re){
    // note, execution order matters!
    auto qm=qapplication();
    // thread safe!
    qm->postEvent(qm,new AnyQAppLambdaEvent(re));
    // will return before event is executed
}

void run_in_gui_thread_blocking(AnyQAppLambda* re){
    std::atomic<bool> done=false;
    // note, execution order matters!
    auto qm=qapplication();
    // thread safe!
    qm->postEvent(qm,new BlockingEvent(re,&done));
    while(!done){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    }
}

void quit(){
    auto app=qapplication_manager()->app;
    run_in_gui_thread_blocking(new QAppLambda([app](){app->quit();}));
}

