#ifndef EVMU_CONTEXT_HPP
#define EVMU_CONTEXT_HPP

#include <gimbal/gimbal_context.h>
#include <gimbal/gimbal_types.hpp>

namespace gimbal {

    template<typename H>
    class HandleObject {
    protected:
        H _hHandle;

    void* getUserdata(void) const;
    void setUserdata(void*) const;

    public:

        HandleObject(H&& other);

        H getHandle(void) const;
        bool isValid(void) const;

        operator H() const;
    };

    enum class Result: uint32_t {
        Unknown,
        Success,
        Failure

    };

    class ResultException {
    private:
        Result result;
    public:
        //stringify


    };


    typedef GBL_RESULT (*GblExtLogWriteFn)(void*, GBL_LOG_LEVEL, const char*, va_list);
    typedef GBL_RESULT (*GblExtLogPushFn)(void*);
    typedef GBL_RESULT (*GblExtLogPopFn)(void*, uint32_t);

    //Custom allocators
    typedef GBL_RESULT (*GblExtMallocFn)(void*, GblSize, GblSize, void**);
    typedef GBL_RESULT (*GblExtReallocFn)(void*, const void*, GblSize, GblSize, void**);
    typedef GBL_RESULT (*GblExtFreeFn)(void*, void*);

    typedef struct GblContextCreateInfo {
        void* pUserdata;

        struct {
            GblExtLogWriteFn      pFnWrite;
            GblExtLogPushFn       pFnPush;
            GblExtLogPopFn        pFnPop;
        }                      logCallbacks;

        struct {
            GblExtMallocFn        pFnMalloc;
            GblExtReallocFn       pFnRealloc;
            GblExtFreeFn          pFnFree;
        }                      allocCallbacks;

        GblVersion             versionMin;
    } GblContextCreateInfo;

    class Error: public GblError {

    };

    class Context: public HandleObject<GblContext> {

        static Context create();


        Context(GblContext&& hContext);
        Context(Context&& );
    public:

        const Error&    getLastError();


        virtual void    logWrite(LogLevel level, const char* pFmt, va_list args) throw();
        virtual void    logPush(void) throw();
        virtual void    logPop(uint32_t count) throw();

        virtual void*   memAlloc(GblSize size, GblSize alignment) throw();
        virtual void*   memRealloc(void* pPtr, GblSize size, GblSize alignment) throw();
        virtual void    memFree(void* pPtr) throw();
    };


    typedef EVMU_RESULT         (*EvmuFileOpenFn)(void*, const char*, const char*, EvmuFile*);
    typedef EVMU_RESULT         (*EvmuFileCloseFn)(EvmuFile, void*);
    typedef EVMU_RESULT         (*EvmuFileReadFn)(EvmuFile, void*, void*, EvmuSize*);
    typedef EVMU_RESULT         (*EvmuFileWriteFn)(EvmuFile, void*, const void*, EvmuSize*);
    typedef EVMU_RESULT         (*EvmuFileLengthFn)(EvmuFile, void*, EvmuSize*);

    namespace evmu {

    class Device:: public HandleObject<EvmuDevice> {
    public:

        bool        addPeripheral(Peripheral* pPeripheral);
        Peripheral* getPeripheral(const char* pName);
        bool        removePeripheral

        virtual void reset(void);
        virtual void saveState(void);
        virtual void loadState(void);
        virtual void eventHandler(Event* pEvent);;

    };



    class Context: public Gimbal::Context {

        Context* create();


    public:

        virtual void run(GblSize ticks);
        virtual void reset(void);
        virtual void saveState(void);
        virtual void loadState(void);

        virtual bool addDevice(Device* pDevice) throw();
        virtual bool removeDevice(Device* pDevice) throw();

        Gimbal::VectorRef<Device*> getDevices(void) const throw();


        virtual File*       fileOpen(const char*, const char*) throw();
        virtual void        fileClose(File*) throw();
        virtual EvmuSize    fileRead(File* void*, EvmuSize*) throw();
        virtual EvmuSize    fileWrite(File* const void*, EvmuSize*) throw();
        virtual EvmuSize    fileLength(File*) throw();

        virtual GblSize     timeTicksMs(void) throw();
        virtual std::time   timeCurrent(void) throw();

        virtual bool        buttonPressed(Button button) throw();

        virtual void        wavStreamPlay() throw();

        virtual bool        eventHandler(Event* pEvent) throw();

        virtual void        apiCookiePush(void) throw();
        virtual void        apiCookiePop(void) throw();


        virtual const Gimbal::Metatype*        metatypeInfo(MetaType type);


    };

    //subclass me for shared standalone libGyro builds?
    class Simulation: public Context {
    public:
        virtual void exec(void);
        virtual void update(GblSize ticks);
        virtual void draw();
        virtual void pollInput();

        virtual void drawPixel();

    };





    }


}



#endif // EVMU_CONTEXT_HPP
