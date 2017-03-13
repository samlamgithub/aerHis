package ac.uk.imperial.aer;

import android.app.Activity;
import android.content.res.AssetManager;
import android.util.Log;

/**
 * JNI Interface
 * @author bjm113
 *
 */

public class JNIInterface {
    static {
        System.loadLibrary("aer");
      }
      
      // Camera interface functions
      public static native boolean cameraInitialise(Activity activity,AssetManager manager);
      public static native void cameraDestroy();

      public static native boolean cameraConnect();
      public static native void cameraDisconnect();

      public static native void cameraSetDisplayViewPort(int width, int height);
      public static native void cameraRender();

      // Imu interface
      public static native void imuOnPause();
      public static native void imuOnResume();

      // Gui interface
      public static native boolean guiInitialise(CameraOverlay gui);
}
