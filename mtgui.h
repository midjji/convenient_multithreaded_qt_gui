#pragma once
/* ********************************* FILE ************************************/
/** \file    mtgui.h
 *
 * \brief    Hide away QApplication in its own thread and provide convenient run in gui thread helpers
 *
 *
 * Issues:
 * - backed by hidden global singletons that initialize on first request, though the QApplication singleton is still a mess.
 * - QApplication must be created before the first related qobject, so if needed ask for the instance.
 *
 * // copy into the lambda
 * run_in_gui_thread(new AnyQAppLambda([arg1, arg2](){do_something(arg1, arg2)})
 * // block to ensure scope remains valid
 * run_in_gui_thread_blocking(new AnyQAppLambda([&arg1, &arg2](){do_something(arg1, arg2);})
 *
 * run_in_gui_thread(new AnyQAppLambda([](function_ptr, Args...){function_ptr(args);},Args... args)
 *
 *
 * \remark
 * - c++17, though easy to downgrade to c++11 if needed,
 * - It may be the case that ios gui explicitly requires qapplication to be in the main thread,
 *       in this case, you must create and exec QApplication in main, after which all this still works perfectly,
 *       which is good for making apis simple
 *
 * \author   Mikael Persson
 *
 ******************************************************************************/

#include <tuple>
struct AnyQAppLambda;
struct QCoreApplication;




/**
 * @brief run_in_gui_thread
 * @param RunEvent* re
 *
 * run_in_gui_thread(RunEventImpl([](Args... args){
 * do_something(args...); new QMainWindow(), window.show() or something... },
 *   args...);
 * \note
 * - Arguments must exist latter and be accessible in a thread safe way from the gui thread.
 *  The simplest way is to copy them.
 * - raw pointers and references are a bad idea.
 * - shared ptrs work wonders.
 * - thread safe, call from anywhere, no need to setup a QApplication, but it works if you have one.
 *
 * run must take a short time only, no heavy work here. But setting up a few windows is fine.
 *
 */
void run_in_gui_thread(AnyQAppLambda* re);

/**
 * @brief run_in_gui_thread_blocking
 * @param re
 *  returns only after the runevent has completed, this mean local capture is fine.
 * \note
 * -This is the recommended way unless you know what you are doing.
 *
 * run_in_gui_thread_blocking(new RunEventImpl([&](){Do stuff with local references... }))
 */
void run_in_gui_thread_blocking(AnyQAppLambda* re);

/**
 * @brief The AnyQAppLambda struct
 *
 *
 *
 *
 *
 */
struct AnyQAppLambda{
    virtual void run()=0;
    virtual ~AnyQAppLambda(){}
};
template<class Lambda, class... Args>

/**
 * @brief The QAppLambda struct,
 *
 * This lets you use a function pointer instead of a closure if you wish.
 * the closure is usually enough though.
 *
*/
struct QAppLambda: public AnyQAppLambda
{
    //  // note do not capture context, all args must exist when called later,
    /**
       * @brief RunEvent
       * @param lambda [](Args&...) {}
       * @param args
       *
       * \note
       * - to minimize error risk, do not capture context and deep copy all arguments.
       * - yes wrapping this class in a function is natural and should be default,
       *      but that causes compiler to exceed max template recursion depth
       */
    QAppLambda(Lambda lambda, Args... args):AnyQAppLambda(),
        lambda(lambda), args(std::make_tuple(args...)){}


    void run() override {
        run_impl(std::make_index_sequence<sizeof...(Args)>());
    }

private:
    Lambda lambda;
    std::tuple<Args...> args;
    template<std::size_t... I>
    void run_impl(std::index_sequence<I...>){
        //cout<<"run_impl"<<endl;
        lambda(std::get<I>(args)...);
    }
};

/**
 * @brief qapplication
 * @return a pointer to the QApplication singleton instance,
 *
 * Unlike QApplication::instance(),
 * this will create and exec a QApplication if one is not available
 * Note returns QCoreApplication if that was was started first.
 *
 * i, strs are used to initialize if this creates the QApplication, otherwize discarded
 *
 */
QCoreApplication* qapplication(int argc=0, char** argv=nullptr);
void wait_for_qapp_to_finish();
// stops the QApp, from anywhere, usually not needed to be called explicitly
void quit();


// for opencv,//TODO
unsigned char wait_key();

