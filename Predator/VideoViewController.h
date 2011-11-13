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
    UIView *_rect;
    NSTimer *timer;
    
    double						startTime;
	double						endTime;
}

@property (nonatomic, retain) UIImageView *imageView;
@property (nonatomic, retain) UIView *rect;

- (void)initVideo;
- (void)startVideo;
- (IBAction) startVideo:(id) sender;

@end
