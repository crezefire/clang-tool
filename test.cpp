//BUILD DIR
#include <string>

namespace Eegeo {
    namespace Api {

        typedef void(*callback)(int, float);

        //struct tag { int x; int y; int z; };
        typedef const int t0;
        typedef t0 t1;

        class EegeoCameraApi {
        public:
            EegeoCameraApi();

            callback OnUpdate(float deltaSeconds);

            const bool SetView(float* out_configs, const std::string& name);

            void SetViewToBounds(double* northEast,
                float southWest[3],
                int asdf);

            t1 StopTransition();
            EegeoCameraApi* TryInterruptTransition();

            const long int CheckToNotifyZoomStart();

            void SetEventCallback(callback cback);

            float GetDistanceToInterest(double altitude, void(*onEvent)(int, float));

            float* GetPitchDegrees(t1 something);

        private:
        };
    }
}
