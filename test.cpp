//BUILD DIR
#include <string>

namespace Eegeo {
    namespace Api {

        typedef void(*callback)(int, float);

        struct Boo {};
        typedef const Boo t0;
        typedef t0 t1;


        class EegeoCameraApi {
        public:
            EegeoCameraApi();

            void SomethingHere() {

            }

            callback OnUpdate(const float deltaSeconds) const;

            const bool SetView(float* out_configs, const std::string& name);

            static void SetViewToBounds(double* northEast, float southWest[3], const bool asdf);

            t1 StopTransition();
            EegeoCameraApi* TryInterruptTransition();

            const long int CheckToNotifyZoomStart(EegeoCameraApi* ptr);

            Boo SetEventCallback(callback cback);

            float GetDistanceToInterest(double altitude, void(*onEvent)(int, float));

            float* GetPitchDegrees(t1 something);

        private:
        };
    }
}
