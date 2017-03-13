package ac.uk.imperial.aer;

import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
/**
 * GLSurfaceRenderer renders graphic content.
 */
public class GLSurfaceRenderer implements GLSurfaceView.Renderer {

    private MainActivity mMainActivity;
    private CameraOverlay mOverlay;

    public GLSurfaceRenderer(MainActivity mainActivity,CameraOverlay overlay) {
        mMainActivity = mainActivity;
        mOverlay = overlay;
    }

    public void onDrawFrame(GL10 gl) {
        JNIInterface.cameraRender();
        mOverlay.requestDraw();
    }

    public void onSurfaceChanged(GL10 gl, int width, int height) {
        JNIInterface.cameraSetDisplayViewPort(width, height);
    }

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        mMainActivity.surfaceCreated();
    }
}