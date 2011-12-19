//
//  CameraViewController.h
//  Predator
//
//  Created by admin on 10/25/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef __CAMERA__
#define __CAMERA__
#import <AVFoundation/AVFoundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreVideo/CoreVideo.h>
#import <CoreMedia/CoreMedia.h>

#import <UIKit/UIKit.h>
@interface CameraViewController : UIViewController<AVCaptureVideoDataOutputSampleBufferDelegate> {
    AVCaptureSession *_captureSession;
	IBOutlet UIImageView *_imageView;
    UIView *_rect;
	UIImageView *_prevLayer;
	UILabel *_fpsLabel;
    
    double						startTime;
	double						endTime;
	double						fpsAverage;
	double						fpsAverageAgingFactor;
	int							framesInSecond;


}

@property (nonatomic, retain) AVCaptureSession *captureSession;
@property (nonatomic, retain) UIImageView *imageView;
@property (nonatomic, retain) UIView *rect;
@property (nonatomic, retain) UIImageView *prevLayer;
@property (nonatomic, retain) UILabel *fpsLabel;

- (void)initCapture;
- (void)calc;
- (IBAction) handleGesture: (UIGestureRecognizer *) sender;
- (IBAction) btnclicked:(id) sender;

+ (void) toggleFps;
+ (void) toggleLearning;
+ (void) setMinLearn: (float) value;
+ (void) setMinTrack: (float) value;
+ (void) setMinScale: (float) value;
+ (void) setMaxScale: (float) value;
+ (void) setWidthSteps: (int) value;
+ (void) setHeightSteps: (int) value;
+ (void) setNumFeatures: (int) value;
+ (void) setNumTrees: (int) value;

@end
#endif