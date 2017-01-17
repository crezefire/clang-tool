namespace Eegeo {
    namespace Api {

        struct tag {};
        typedef tag t0;
        typedef t0 t1;

        class EegeoCameraApi {
        public:
            EegeoCameraApi();

            int OnUpdate(float deltaSeconds);

            bool SetView(bool animated, double latDegrees, double longDegrees,
                double altitude);

            void SetViewToBounds(double* northEast,
                double* southWest,
                bool animated, bool allowInterruption);

            t1 StopTransition();
            const EegeoCameraApi* TryInterruptTransition();

            EegeoCameraApi CheckToNotifyZoomStart();

            //void SetEventCallback(TCameraEventCallback callback);

            float* GetDistanceToInterest();
            //Eegeo::dv3 GetCameraInterestPoint();
            const float* GetPitchDegrees();

        private:
        };
    }
}

// ArrayType
// AtomicType
// AutoType
// BuiltinType
// DecltypeType
// FunctionType
// PointerType
// ReferenceType
// TagType (class + enum)
// TypedefType
