#pragma once

namespace Eegeo
{
    namespace Api
    {
        typedef void(*TCameraEventCallback)(int);
        enum CameraEventType
        {
            Move,
            MoveStart,
            MoveEnd,
            Drag,
            DragStart,
            DragEnd,
            Pan,
            PanStart,
            PanEnd,
            Rotate,
            RotateStart,
            RotateEnd,
            Tilt,
            TiltStart,
            TiltEnd,
            Zoom,
            ZoomStart,
            ZoomEnd,
            TransitionStart,
            TransitionEnd
        };

        class EegeoCameraApi
        {
        public:
            EegeoCameraApi(
                float gpsGlobeCameraController,
                float interiorsCameraController,
                const int& interiorSelectionModel);

            void OnUpdate(float deltaSeconds);

            bool SetView(
                bool animated,
                double latDegrees, double longDegrees, double altitude, bool modifyPosition,
                double distance, bool modifyDistance,
                double headingDegrees, bool modifyHeading,
                double tiltDegrees, bool modifyTilt,
                double transitionDurationSeconds, bool hasTransitionDuration,
                bool jumpIfFarAway,
                bool allowInterruption
            );

            void SetViewToBounds(
                const double& northEast,
                const double& southWest,
                bool animated,
                bool allowInterruption);

            void StopTransition();
            void TryInterruptTransition();

            void CheckToNotifyZoomStart();

            void SetEventCallback(TCameraEventCallback callback);

            float GetDistanceToInterest();
            double* GetCameraInterestPoint();
            float GetPitchDegrees();

        private:
            void HandleTransitionStopped();

            void CheckToNotifyPan();
            void CheckToNotifyRotate();
            void CheckToNotifyTilt();
            void CheckToNotifyZoom();

            void NotifyEventCallback(CameraEventType eventType) const;

            TCameraEventCallback m_eventCallback;

            bool m_shouldNotifyZoom;
            bool m_wasDragging;
            bool m_wasPanning;
            bool m_wasRotating;
            bool m_wasTilting;
            bool m_wasZooming;
            float m_previousCameraRightVector[3];
            float m_previousTiltDegrees;
            float m_previousDistanceToInterest;
        };
    }
}
