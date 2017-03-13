package ac.uk.imperial.aer;

import java.util.ArrayList;

import android.content.Context;
import android.graphics.Canvas;
import android.util.Log;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.PorterDuff.Mode;

class MarkerOverlay {
  public String label;
  public int label_x;
  public int label_y;
  public int x1;
  public int x2;
  public int x3;
  public int x4;
  public int y1;
  public int y2;
  public int y3;
  public int y4;

  public MarkerOverlay(int label_, int x1_, int y1_, int x2_, int y2_, 
                       int x3_, int y3_, int x4_, int y4_) {
    label = String.valueOf(label_);
    label_x = (x1_ + x2_ + x3_ + x4_) / 4;
    label_y = (y1_ + y2_ + y3_ + y4_) / 4;
    x1 = x1_;
    x2 = x2_;
    x3 = x3_;
    x4 = x4_;
    y1 = y1_;
    y2 = y2_;
    y3 = y3_;
    y4 = y4_;
  }
}

class CameraOverlay extends SurfaceView implements SurfaceHolder.Callback {
  private SurfaceHolder mHolder;
  private Paint mPaint;
  private ArrayList<MarkerOverlay> mMarkerOverlays;
  private int mCameraFrameHeight;
  private int mCameraFrameWidth;

  public CameraOverlay(MainActivity mainActivity) {
      super(mainActivity);
      setZOrderOnTop(true);
      mHolder = getHolder();
      mHolder.addCallback(this);
      mHolder.setFormat(PixelFormat.TRANSPARENT);
      mPaint = new Paint();
      mPaint.setColor(Color.GREEN);
      mPaint.setTextSize(30);
      mPaint.setTextAlign(Paint.Align.CENTER);
      mMarkerOverlays = new ArrayList<MarkerOverlay>();
      mCameraFrameHeight = 0;
      mCameraFrameWidth = 0;
      JNIInterface.guiInitialise(this);
  }

  public void clearMarkers() {
    mMarkerOverlays.clear();
  }

  //NOTE This can only safely called from the rendering thread
  public void addMarker(int label_, int x1_, int y1_, int x2_, int y2_, int x3_, int y3_, int x4_, int y4_) {
    MarkerOverlay overlay = new MarkerOverlay(label_,x1_,y1_,x2_,y2_,x3_,y3_,x4_,y4_);
    mMarkerOverlays.add(overlay);
  }

  public void drawMarkers(Canvas canvas) {
    double width_scale = (double) canvas.getWidth() / mCameraFrameWidth;
    double width_constant = 0;
    double height_scale = (double) canvas.getHeight() /  mCameraFrameHeight;
    double height_constant = 0;
    for(MarkerOverlay overlay: mMarkerOverlays) {
      canvas.drawText(overlay.label, (int)(overlay.label_x * width_scale),
          (int) (overlay.label_y * height_scale)- ((mPaint.descent() + mPaint.ascent()) / 2), mPaint);
      canvas.drawCircle((int)(overlay.x1 * width_scale + width_constant),(int)(overlay.y1 * height_scale + height_constant),5,mPaint);
      canvas.drawCircle((int)(overlay.x2 * width_scale + width_constant),(int)(overlay.y2 * height_scale + height_constant),5,mPaint);
      canvas.drawCircle((int)(overlay.x3 * width_scale + width_constant),(int)(overlay.y3 * height_scale + height_constant),5,mPaint);
      canvas.drawCircle((int)(overlay.x4 * width_scale + width_constant),(int)(overlay.y4 * height_scale + height_constant),5,mPaint);
    }
    clearMarkers();
  }

  public void surfaceCreated(SurfaceHolder holder) {
  }

  public void surfaceDestroyed(SurfaceHolder holder) {
  }

  public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
  }
  
  public void requestDraw() {
    Canvas canvas = mHolder.lockCanvas();
    canvas.drawColor(Color.TRANSPARENT, Mode.CLEAR);
    drawMarkers(canvas);
    mHolder.unlockCanvasAndPost(canvas);
  }
  
  public void setCameraSize(int width, int height) {
    if (width > 0 && height > 0) {
      mCameraFrameHeight = height;
      mCameraFrameWidth = width;
    }
  }
}