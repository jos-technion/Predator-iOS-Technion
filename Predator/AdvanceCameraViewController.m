//
//  AdvanceCameraViewController.m
//  Predator
//
//  Created by admin on 11/6/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "AdvanceCameraViewController.h"
#import "CameraViewController.h"

static bool show_fps = true;
static bool online_learning = false;
static float min_learn_value = 0.12;
static float min_track_value = 0.05;
static float min_scale_value = 0.1;
static float max_scale_value = 0.5;
static int num_features_value = 13;
static int num_trees_value = 10;
static int width_steps_value = 30;
static int height_steps_value = 30;




@implementation AdvanceCameraViewController



@synthesize fpsSwitch = _fpsSwitch;
@synthesize min_learn = _min_learn;
@synthesize min_track = _min_track;
@synthesize min_scale = _min_scale;
@synthesize max_scale = _max_scale;
@synthesize with_steps = _with_steps;
@synthesize height_steps = _height_steps;
@synthesize num_features = _num_features;
@synthesize num_trees = _num_trees;
@synthesize onlineLearning = _onlineLearning;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)dealloc
{
    [super dealloc];
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
    [super viewDidLoad];

    [self.fpsSwitch setOn: show_fps];
    [self.min_learn setValue:min_learn_value animated:YES];
    [self.min_track setValue:min_track_value];
    [self.min_scale setValue:min_scale_value];
    [self.max_scale  setValue:max_scale_value];
    [self.with_steps setValue:width_steps_value];
    [self.height_steps  setValue:height_steps_value];
    [self.num_features setValue:num_features_value];
    [self.num_trees  setValue:num_trees_value];
    
    
    // Do any additional setup after loading the view from its nib.
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
	return YES;
}
- (IBAction) fpsButton:(id)sender {
    show_fps = !show_fps;
    [CameraViewController toggleFps];
}
- (IBAction) minLearn:(UISlider*) sender {
    min_learn_value = sender.value;
    [CameraViewController setMinLearn: min_learn_value];
}
- (IBAction) minTrack:(UISlider*) sender {
    min_track_value = sender.value;   
    [CameraViewController setMinTrack: min_track_value]; 
}
- (IBAction) minScale:(UISlider*) sender {
   min_scale_value = sender.value;   
    [CameraViewController setMinScale:  min_scale_value];
}
- (IBAction) maxScale:(UISlider*) sender {
   max_scale_value =  sender.value;   
    [CameraViewController setMaxScale: max_scale_value]; 
}
- (IBAction) withSteps:(UISlider*) sender {
    width_steps_value = sender.value;  
    [CameraViewController setWidthSteps: width_steps_value]; 
}
- (IBAction) heightSteps:(UISlider*) sender {
    height_steps_value = sender.value;   
    [CameraViewController setHeightSteps: height_steps_value] ;
}
- (IBAction) numFeatures:(UISlider*) sender {
    num_features_value = sender.value;   
    [CameraViewController setNumFeatures: num_features_value]; 
}
- (IBAction) numTrees:(UISlider*) sender {
    num_trees_value = sender.value;
    [CameraViewController setNumTrees: num_trees_value]; 
}
- (IBAction) onlineLearningButton:(UISwitch*)sender {
    online_learning = !online_learning;
    [CameraViewController toggleLearning];

}
@end
