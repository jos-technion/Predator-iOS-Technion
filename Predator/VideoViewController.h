//
//  VideoViewController.h
//  Predator
//
//  Created by admin on 10/26/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface VideoViewController : UIViewController {
    
	IBOutlet UIImageView *_imageView;
    IBOutlet UISwitch *_learningSwitch;
    UIView *_rect;
    NSTimer *timer;
    
    double						startTime;
	double						endTime;
}

@property (nonatomic, retain) UIImageView *imageView;
@property (nonatomic, retain) UIView *rect;
@property (nonatomic, retain) UISwitch *learningSwitch;
- (void)initVideo;
- (void)startVideo;
- (IBAction) startVideo:(id) sender;
- (IBAction) onlineLearning:(id) sender;

@end
