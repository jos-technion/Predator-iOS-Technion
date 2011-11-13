//
//  AdvanceCameraViewController.h
//  Predator
//
//  Created by admin on 11/6/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface AdvanceCameraViewController : UIViewController {
    IBOutlet UISwitch *_fpsSwitch;
    IBOutlet UISlider *_min_learn;
    IBOutlet UISlider *_min_track;
    IBOutlet UISlider *_min_scale;
    IBOutlet UISlider *_max_scale;
    IBOutlet UISlider *_with_steps;
    IBOutlet UISlider *_height_steps;
    IBOutlet UISlider *_num_features;
    IBOutlet UISlider *_num_trees;


}
@property (nonatomic,retain) UISwitch *fpsSwitch;
@property (nonatomic,retain) UISlider *min_learn;
@property (nonatomic,retain) UISlider *min_track;
@property (nonatomic,retain) UISlider *min_scale;
@property (nonatomic,retain) UISlider *max_scale;
@property (nonatomic,retain) UISlider *with_steps;
@property (nonatomic,retain) UISlider *height_steps;
@property (nonatomic,retain) UISlider *num_features;
@property (nonatomic,retain) UISlider *num_trees;
- (IBAction) fpsButton:(id) sender;
- (IBAction) minLearn:(UISlider*) sender;
- (IBAction) minTrack:(id) sender;
- (IBAction) minScale:(id) sender;
- (IBAction) maxScale:(id) sender;
- (IBAction) withSteps:(id) sender;
- (IBAction) heightSteps:(id) sender;
- (IBAction) numFeatures:(id) sender;
- (IBAction) numTrees:(id) sender;
@end
