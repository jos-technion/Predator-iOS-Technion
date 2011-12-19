//
//  VideoViewController.m
//  Predator
//
//  Created by admin on 10/26/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//
#include "Tld.h"

#import "VideoViewController.h"
#import "PredatorViewController.h"
#import "UIImage+Pixels.h"
#import "NSInvocation+CWVariableArguments.h"

Tld *tld2 = 0;
NSInteger fps2 = 0;

UIImage *lastImage2;
//static float width;
static bool is_initialized = false;
//static float height;
static float image_height;
static float image_width;
static double bbox[4];
static double output_bbox[4];
static bool learning = true;

static int nTREES    = 15;
static int nFEATS     = 13;

static float MIN_FEATURE_SCALE = 0.1f;
static float MAX_FEATURE_SCALE = 0.5f;

static double MIN_LEARNING_CONF =  0.12f;

static double MIN_REINIT_CONF   =  0.35f;

static double MIN_TRACKING_CONF =  0.05f;

static int WIDTH_STEPS          = 100;
static int HEIGHT_STEPS         = 40;




static void neon_asm_convert(uint8_t * __restrict dest, uint8_t * __restrict src, int numPixels)
{
	__asm__ volatile("lsr          %2, %2, #3      \n"
					 "# build the three constants: \n"
					 "mov         r4, #76          \n" // red channel multiplier
					 "mov         r5, #150         \n" // Green channel multiplier
					 "mov         r6, #29          \n" // blue channel multiplier
					 "vdup.8      d4, r4           \n"
					 "vdup.8      d5, r5           \n"
					 "vdup.8      d6, r6           \n"
					 "0:						   \n"
					 "# load 8 pixels:             \n"
					 "vld4.8      {d0-d3}, [%1]!   \n"
					 "# do the weight average:     \n"
					 "vmull.u8    q7, d0, d4       \n"
					 "vmlal.u8    q7, d1, d5       \n"
					 "vmlal.u8    q7, d2, d6       \n"
					 "# shift and store:           \n"
					 "vshrn.u16   d7, q7, #8       \n" // Divide q3 by 256 and store in the d7
					 "vst1.8      {d7}, [%0]!      \n"
					 "subs        %2, %2, #1       \n" // Decrement iteration count
					 "bne         0b            \n" // Repeat unil iteration count is not zero
					 :
					 : "r"(dest), "r"(src), "r"(numPixels)
					 : "r4", "r5", "r6"
					 );
}

@implementation VideoViewController

@synthesize imageView = _imageView;
@synthesize rect = _rect;
@synthesize learningSwitch = _learningSwitch;

PredatorViewController *mymainViewController;


- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
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

-(void) initVideo {
    
    // number of ferns (trees) in the classifer
  
    self.rect=[[UIView alloc] init];
    [self.imageView addSubview:self.rect];

    bbox[0] = 288;
    bbox[1] = 36;
    bbox[2] = 313-288;
    bbox[3] = 78-36;
    lastImage2 =[UIImage imageNamed: @"1.png"];
      image_width = lastImage2.size.width;
    image_height = lastImage2.size.height;
     unsigned char *pixelData = [lastImage2 rgbaPixels];
    uint8_t* baseAddressGray = (uint8_t*) malloc(470*310);
    neon_asm_convert(baseAddressGray, pixelData, 470*310); // 1 ms
  	
	CGColorSpaceRef colorSpaceGray = CGColorSpaceCreateDeviceGray();
	CGContextRef newContextGray = CGBitmapContextCreate(baseAddressGray, 470, 310, 8, 470, colorSpaceGray, kCGImageAlphaNone);
        
	//Create CgImageRef from cgcontext
	CGImageRef GrayImage = CGBitmapContextCreateImage(newContextGray);
	
	//convert to uiimage
	UIImage *img2= [UIImage imageWithCGImage:GrayImage scale:1.0 orientation:UIImageOrientationUp];
    [self.imageView setImage: img2];
 
    output_bbox[0] = 0; output_bbox[1] = 0; output_bbox[2] = 0; output_bbox[3] = 0;
    tld2 = new Tld(470, 310, baseAddressGray, bbox, nTREES, nFEATS, MIN_FEATURE_SCALE, MAX_FEATURE_SCALE , WIDTH_STEPS, HEIGHT_STEPS, true, true);
    
    self.rect.layer.borderColor = [UIColor redColor].CGColor;
    self.rect.layer.borderWidth = 3.0f;
    self.rect.frame = self.imageView.frame;
    self.rect.layer.frame = CGRectMake(bbox[0],bbox[1],bbox[2],bbox[3]);
    if(! is_initialized) {
          timer = [NSTimer scheduledTimerWithTimeInterval:0.001
                                                  target:self
                                                selector:@selector(startVideo)
                                                userInfo:nil
                                                 repeats:YES];

        is_initialized = true;
    }
    free(baseAddressGray);
  	CGColorSpaceRelease(colorSpaceGray);
    CGContextRelease(newContextGray);

}


- (void)startVideo {
    //470x310
    static int i=1;
    { 
        i++;
        [timer invalidate];
        NSString *pictureName = [NSString stringWithFormat:@"%d.png", i];
        lastImage2 = [UIImage imageNamed:pictureName];
        
        unsigned char *pixelData = [lastImage2 rgbaPixels];
        uint8_t* baseAddressGray = (uint8_t*) malloc(470*310);
        neon_asm_convert(baseAddressGray, pixelData, 470*310); // 1 ms
        CGColorSpaceRef colorSpaceGray = CGColorSpaceCreateDeviceGray();

        CGContextRef newContextGray = CGBitmapContextCreate(baseAddressGray, 470, 310, 8, 470, colorSpaceGray, kCGImageAlphaNone);
      
        //Create CgImageRef from cgcontext
        CGImageRef GrayImage = CGBitmapContextCreateImage(newContextGray);
        
        //convert to uiimage
        UIImage *img2= [UIImage imageWithCGImage:GrayImage scale:1.0 orientation:UIImageOrientationUp];
        [self.imageView setImage: img2];
        printf("Sending: %d\n",learning);
        tld2->tldProcessFrame(470, 310, baseAddressGray, bbox, output_bbox, MIN_TRACKING_CONF,  MIN_REINIT_CONF,  MIN_LEARNING_CONF, true, learning);
    
        bbox[0] = output_bbox[0];
        bbox[1] = output_bbox[1];
        bbox[2] = output_bbox[2];
        bbox[3] = output_bbox[3];
        if(output_bbox[2] > 0 && output_bbox[3] > 0) {
            [[NSInvocation invocationWithTarget:self.rect.layer selector:@selector(setHidden:) retainArguments:NO, NO] invokeOnMainThreadWaitUntilDone:NO];
            [[NSInvocation invocationWithTarget:self.rect.layer selector:@selector(setFrame:) retainArguments:NO, CGRectMake(bbox[0],bbox[1],bbox[2],bbox[3])] invokeOnMainThreadWaitUntilDone:NO];
        } else {
            [[NSInvocation invocationWithTarget:self.rect.layer selector:@selector(setHidden:) retainArguments:NO, YES] invokeOnMainThreadWaitUntilDone:NO];
        }
        free(baseAddressGray);
        CGColorSpaceRelease(colorSpaceGray);
        CGContextRelease(newContextGray);
        timer = [NSTimer scheduledTimerWithTimeInterval:0.001
                                                 target:self
                                               selector:@selector(startVideo)
                                               userInfo:nil
                                                repeats:YES];
        if (i==70) [timer invalidate];
        
   }
    
    
}
- (IBAction) startVideo:(id) sender {
    
   timer = [NSTimer scheduledTimerWithTimeInterval:0.2
                                     target:self
                                   selector:@selector(initVideo)
                                   userInfo:nil
                                    repeats:NO];
   

}
-(void) viewWillDisappear:(BOOL)animated {
    if ([self.navigationController.viewControllers indexOfObject:self]==NSNotFound) {
        [timer invalidate];
        delete tld2;
        is_initialized = false;
        tld2 = 0;
    }
    [super viewWillDisappear:animated];
}
-(IBAction) onlineLearning:(id)sender {
    learning = !learning;
}
@end
