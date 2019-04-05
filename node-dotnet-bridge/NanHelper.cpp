= Javascript =========================================================================================

function runAsync() {
  // how many batches should we split the work in to?
  var batches = process.argv[3] || 16;
  var ended = 0;
  var total = 0;
  var start = Date.now();

  function done (err, result) {
    total += result;
    
    // have all the batches finished executing?
    if (++ended === batches) {
      printResult('Async', total / batches, Date.now() - start);
    }
  }

  // for each batch of work, request an async Estimate() for
  // a portion of the total number of calculations
  for (var i = 0; i < batches; i++) {
    addon.calculateAsync(calculations / batches, done);
  }
}

= C++-Gegenstück ==========================================================================================

NAN_METHOD(CalculateAsync) {
    int points = To<int>(info[0]).FromJust();
    Callback *callback = new Callback(To<Function>(info[1]).ToLocalChecked());

    AsyncQueueWorker (new PiWorker(callback, points));
}

= Globale Funktion Execute in Worker Thread ==========================================================================================

inline void AsyncExecute (uv_work_t* req) {
  AsyncWorker *worker = static_cast<AsyncWorker*>(req->data);
  worker->Execute();
}

= Globale Funcktion Rückkehr in UI-Thread ==========================================================================================

inline void AsyncExecuteComplete (uv_work_t* req) {
  AsyncWorker* worker = static_cast<AsyncWorker*>(req->data);
  worker->WorkComplete();
  worker->Destroy();
}

= Globale Funktion Javascript-Gegenstück ==========================================================================================

inline void AsyncQueueWorker(AsyncWorker* worker) {
    uv_queue_work(
        GetCurrentEventLoop()
        , &worker->request
        , AsyncExecute
        , reinterpret_cast<uv_after_work_cb>(AsyncExecuteComplete)
    );
}

= Abgeleiteter AsycWorker ==========================================================================================

class PiWorker : public AsyncWorker {
public:
    PiWorker(Callback *callback, int points)
        : AsyncWorker(callback), points(points), estimate(0) {}
    ~PiWorker() {}

    // Executed inside the worker-thread.
    // It is not safe to access V8, or V8 data structures
    // here, so everything we need for input and output
    // should go on `this`.
    void Execute () {
        estimate = Estimate(points);
    }

    // Executed when the async work is complete
    // this function will be run inside the main event loop
    // so it is safe to use V8 again
    void HandleOKCallback () {
        HandleScope scope;

        Local<Value> argv[] = {
            Null()
            , New<Number>(estimate)
        };

        callback->Call(2, argv, async_resource);
    }

private:
    int points;
    double estimate;
};

= AsycWorker Basis ==========================================================================================

/* abstract */ class AsyncWorker {
public:
    explicit AsyncWorker(Callback *callback_,
                        const char* resource_name = "nan:AsyncWorker")
        : callback(callback_), errmsg_(NULL) {
        request.data = this;

        HandleScope scope;
        v8::Local<v8::Object> obj = New<v8::Object>();
        persistentHandle.Reset(obj);
        async_resource = new AsyncResource(resource_name, obj);
    }

    virtual ~AsyncWorker() {
        HandleScope scope;

        if (!persistentHandle.IsEmpty())
        persistentHandle.Reset();
        delete callback;
        delete[] errmsg_;
        delete async_resource;
    }

    virtual void WorkComplete() {
        HandleScope scope;

        if (errmsg_ == NULL)
        HandleOKCallback();
        else
        HandleErrorCallback();
        delete callback;
        callback = NULL;
    }

    inline void SaveToPersistent(
        const char *key, const v8::Local<v8::Value> &value) {
        HandleScope scope;
        Set(New(persistentHandle), New(key).ToLocalChecked(), value).FromJust();
    }

    inline void SaveToPersistent(
        const v8::Local<v8::String> &key, const v8::Local<v8::Value> &value) {
        HandleScope scope;
        Set(New(persistentHandle), key, value).FromJust();
    }

    inline void SaveToPersistent(
        uint32_t index, const v8::Local<v8::Value> &value) {
        HandleScope scope;
        Set(New(persistentHandle), index, value).FromJust();
    }

    inline v8::Local<v8::Value> GetFromPersistent(const char *key) const {
        EscapableHandleScope scope;
        return scope.Escape(
            Get(New(persistentHandle), New(key).ToLocalChecked())
            .FromMaybe(v8::Local<v8::Value>()));
    }

    inline v8::Local<v8::Value>
    GetFromPersistent(const v8::Local<v8::String> &key) const {
        EscapableHandleScope scope;
        return scope.Escape(
            Get(New(persistentHandle), key)
            .FromMaybe(v8::Local<v8::Value>()));
    }

    inline v8::Local<v8::Value> GetFromPersistent(uint32_t index) const {
        EscapableHandleScope scope;
        return scope.Escape(
            Get(New(persistentHandle), index)
            .FromMaybe(v8::Local<v8::Value>()));
    }

    virtual void Execute() = 0;

    uv_work_t request;

    virtual void Destroy() {
        delete this;
    }

protected:
    Persistent<v8::Object> persistentHandle;
    Callback *callback;
    AsyncResource *async_resource;

    virtual void HandleOKCallback() {
        HandleScope scope;

        callback->Call(0, NULL, async_resource);
    }

    virtual void HandleErrorCallback() {
        HandleScope scope;

        v8::Local<v8::Value> argv[] = {
        v8::Exception::Error(New<v8::String>(ErrorMessage()).ToLocalChecked())
        };
        callback->Call(1, argv, async_resource);
    }

    void SetErrorMessage(const char *msg) {
        delete[] errmsg_;

        size_t size = strlen(msg) + 1;
        errmsg_ = new char[size];
        memcpy(errmsg_, msg, size);
    }

    const char* ErrorMessage() const {
        return errmsg_;
    }

private:
    NAN_DISALLOW_ASSIGN_COPY_MOVE(AsyncWorker)
    char *errmsg_;
};
