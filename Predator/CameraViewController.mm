//
//  CameraViewController.m
//  Predator
//
//  Created by admin on 10/25/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//
#include "Tld.h"
#include "Utils.h"
#include <opencv2/opencv.hpp>
#import "CameraViewController.h"
#import "UIImage+Pixels.h"
#import "NSInvocation+CWVariableArguments.h"
#import "AdvanceCameraViewController.h"
Tld *tld = 0;
NSInteger fps = 0;
UIImage *lastImage;
static float width;
static float height;
static float image_height;
static float image_width;
static bool is_initialized = 0;
static double bbox[4];
static double output_bbox[4];
static double screen[4];

static int nTREES    = 10;
static int nFEAT     = 13;

static float MIN_FEATURE_SCALE = 0.1f;
static float MAX_FEATURE_SCALE = 0.5f;

static double MIN_LEARNING_CONF =  0.2f;

static double MIN_REINIT_CONF   =  0.6f;

static double MIN_TRACKING_CONF =  0.12f;

static int WIDTH_STEPS          = 60;
static int HEIGHT_STEPS         = 60;
static bool show_fps             = true;
static bool use_face_detection = false;
static void screenToBbox(double* screen, double* bbox) {
    
    bbox[0] = screen[0] * 360.0/768.0;
    bbox[1] = screen[1]* 480/1004.0;
    bbox[2] = screen[2] * 360.0/768.0;
    bbox[3] = screen[3]* 480/1004.0;
    
}

static void bboxToScreen(double* bbox, double* screen) {
    screen[0] = bbox[0] * 768.0/360.0;
    screen[1] = bbox[1]* 1004.0/480;
    screen[2] = bbox[2] * 768.0/360.0;
    screen[3] = bbox[3]* 1004.0/480;
    
}


@implementation CameraViewController
 AdvanceCameraViewController* myAdvanceCameraViewController;
@synthesize captureSession = _captureSession;
@synthesize imageView = _imageView;
@synthesize prevLayer = _prevLayer;
@synthesize fpsLabel = _fpsLabel; 
@synthesize rect = _rect;


- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        [self initCapture];
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
    
    
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
    [super viewDidLoad];
    UIBarButtonItem *anotherButton = [[UIBarButtonItem alloc] initWithTitle:@"Advanced Settings" style:UIBarButtonItemStylePlain target:self action:@selector(openAdvanceSettings:)];          
    self.navigationItem.rightBarButtonItem = anotherButton;
    [anotherButton release];
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

- (void)calc
{
	NSLog(@"fps: %i", fps);
	fps = 0;
}


- (void)initCapture {


	
	/*We setup the input*/
	AVCaptureDeviceInput *captureInput = [AVCaptureDeviceInput 
										  deviceInputWithDevice:[AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo] 
										  error:nil];
	/*We setupt the output*/
	AVCaptureVideoDataOutput *captureOutput = [[AVCaptureVideoDataOutput alloc] init];
	/*While a frame is processes in -captureOutput:didOutputSampleBuffer:fromConnection: delegate methods no other frames are added in the queue.
	 If you don't want this behaviour set the property to NO */
	captureOutput.alwaysDiscardsLateVideoFrames = YES; 
	/*We specify a minimum duration for each frame (play with this settings to avoid having too many frames waiting
	 in the queue because it can cause memory issues). It is similar to the inverse of the maximum framerate.
	 In this example we set a min frame duration of 1/10 seconds so a maximum framerate of 10fps. We say that
	 we are not able to process more than 10 frames per second.*/
	//captureOutput.minFrameDuration = CMTimeMake(1, 10);
	
	/*We create a serial queue to handle the processing of our frames*/
	dispatch_queue_t queue;
	queue = dispatch_queue_create("cameraQueue", NULL);
	[captureOutput setSampleBufferDelegate:self queue:queue];
	dispatch_release(queue);
	// Set the video output to store frame in BGRA (It is supposed to be faster)
	
	NSString* key = (NSString*)kCVPixelBufferPixelFormatTypeKey; 
	
	
	NSNumber* value = [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA]; 
	NSDictionary* videoSettings = [NSDictionary dictionaryWithObject:value forKey:key]; 
	[captureOutput setVideoSettings:videoSettings]; 
	/*And we create a capture session*/
	self.captureSession = [[AVCaptureSession alloc] init];
	self.captureSession.sessionPreset = AVCaptureSessionPresetMedium;
	/*We add input and output*/
	[self.captureSession addInput:captureInput];
	[self.captureSession addOutput:captureOutput];
	
		
	CGRect bounds = [UIScreen mainScreen].bounds;
    self.prevLayer = [[UIImageView alloc] init];
	self.prevLayer.frame = CGRectMake(0, 0, 180,250);
    self.prevLayer.backgroundColor = [UIColor redColor];
    self.prevLayer.hidden = YES;

    /*We add the imageView*/
	
	    
    self.imageView.frame = CGRectMake(0, 0, 768, 1024);
	self.imageView.backgroundColor = [UIColor blackColor];
	[self.view addSubview:self.imageView];
    [self.view addSubview:self.prevLayer];
    self.prevLayer.hidden = YES;
    
    /*We add the gesture handler */
    UIPanGestureRecognizer *panGesture = 
    [[UIPanGestureRecognizer alloc]
     initWithTarget:self action:@selector(handleGesture:)];
	
    
	/* Add the fps Label */
	UILabel *fps = [[UILabel alloc] initWithFrame:CGRectMake(bounds.size.width - 100, 0, 100, 20)];
	self.fpsLabel = fps;
	[self.view addSubview:self.fpsLabel];
	[self.view bringSubviewToFront:self.fpsLabel];
	[fps release];
	
	
    self.rect=[[UIView alloc] init];
    [self.imageView addGestureRecognizer:panGesture];
    [panGesture release];
    [self.imageView setUserInteractionEnabled:YES];
    [self.imageView addSubview:self.rect];
	/*We start the capture*/
	[self.captureSession startRunning];
	
}
#include <arm_neon.h>
void neon_convert(uint8_t * __restrict dest, uint8_t * __restrict src, int numPixels)
{
	int i;
	uint8x8_t rfac = vdup_n_u8 (77);
	uint8x8_t gfac = vdup_n_u8 (151);
	uint8x8_t bfac = vdup_n_u8 (28);
	int n = numPixels / 8;
	
	// Convert per eight pixels
	for (i=0; i < n; ++i)
	{
		uint16x8_t  temp;
		uint8x8x4_t rgb  = vld4_u8 (src);
		uint8x8_t result;
		
		temp = vmull_u8 (rgb.val[0],      bfac);
		temp = vmlal_u8 (temp,rgb.val[1], gfac);
		temp = vmlal_u8 (temp,rgb.val[2], rfac);
		
		result = vshrn_n_u16 (temp, 8);
		vst1_u8 (dest, result);
		src  += 8*4;
		dest += 8;
	}
}

static void neon_asm_convert(uint8_t * __restrict dest, uint8_t * __restrict src, int numPixels)
{
	__asm__ volatile("lsr          %2, %2, #3      \n"
					 "# build the three constants: \n"
					 "mov         r4, #28          \n" // Blue channel multiplier
					 "mov         r5, #151         \n" // Green channel multiplier
					 "mov         r6, #77          \n" // Red channel multiplier
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


#pragma mark -
#pragma mark AVCaptureSession delegate
- (void)captureOutput:(AVCaptureOutput *)captureOutput 
didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer 
	   fromConnection:(AVCaptureConnection *)connection 
{ 
    
	// Calculate FPS
	fpsAverageAgingFactor = 0.2;
	framesInSecond++;
	endTime = [[NSDate date] timeIntervalSince1970];
	if (show_fps) {
        self.fpsLabel.hidden = NO;
    } else {
        self.fpsLabel.hidden = YES;
    }
	if (startTime <= 0) {
		startTime = [[NSDate date] timeIntervalSince1970];
	}
	else {
		if (endTime - startTime >= 1) {
			double currentFPS = framesInSecond / (endTime - startTime);
			fpsAverage = fpsAverageAgingFactor * fpsAverage + (1.0 - fpsAverageAgingFactor) * currentFPS;
			startTime = [[NSDate date] timeIntervalSince1970];
			framesInSecond = 0;
		}
		
		[self.fpsLabel performSelectorOnMainThread:@selector(setText:) withObject:[NSString stringWithFormat:@"FPS: %.2f", fpsAverage] waitUntilDone:NO];
	}
	
	/*We create an autorelease pool because as we are not in the main_queue our code is
	 not executed in the main thread. So we have to create an autorelease pool for the thread we are in*/
	
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer); 
    /*Lock the image buffer*/
    CVPixelBufferLockBaseAddress(imageBuffer,0); 
    /*Get information about the image*/
    uint8_t *baseAddress = (uint8_t *)CVPixelBufferGetBaseAddress(imageBuffer); 
    width = CVPixelBufferGetWidth(imageBuffer); 
    height = CVPixelBufferGetHeight(imageBuffer);  
    
	//create memory
    
    
	uint8_t* baseAddressGray = (uint8_t*) malloc(width*height);
    //convert 
	neon_asm_convert(baseAddressGray, baseAddress, width*height); // 1 ms
	    
    //from gray conversion to cgcontext
	CGColorSpaceRef colorSpaceGray = CGColorSpaceCreateDeviceGray();
	CGContextRef newContextGray = CGBitmapContextCreate(baseAddressGray, width, height, 8, width, colorSpaceGray, kCGImageAlphaNone);
    
    
	//Create CgImageRef from cgcontext
	CGImageRef GrayImage = CGBitmapContextCreateImage(newContextGray);
	
	//convert to uiimage
	UIImage *img2= [UIImage imageWithCGImage:GrayImage scale:1.0 orientation:UIImageOrientationRight];
    
    lastImage = img2;
	[self.imageView performSelectorOnMainThread:@selector(setImage:) withObject:img2 waitUntilDone:NO];
	if (use_face_detection && !is_initialized) {
        unsigned char* pixel_data = [lastImage grayscalePixels];
        CvSize *frameSize = (CvSize *)malloc(sizeof(CvSize));
        CvMemStorage            *storage;

        NSString *path = [[NSBundle mainBundle] pathForResource:@"haarcascade_frontalface_alt" ofType:@"xml"];
		CvHaarClassifierCascade* cascade = (CvHaarClassifierCascade*)cvLoad([path cStringUsingEncoding:NSASCIIStringEncoding], NULL, NULL, NULL);
		image_height = lastImage.size.height;
        image_width = lastImage.size.width;

        
        /* setup memory buffer; needed by the face detector */
        storage = cvCreateMemStorage( 0 );
        assert(cascade);
        *frameSize = cvSize(image_width, image_height);
        unsigned char* frame = (unsigned char*) malloc (sizeof(unsigned char) * image_width * image_height);
        for (int i = 0; i < image_height; i++) {
            for (int j = 0; j < image_width; j++) {
                //90 degree rotation
                frame[j*(int)image_width + ((int)image_height-i-1)] = pixel_data[i*(int)image_width+j];
            }
        }

        IplImage * img =  imageFromChar(frame, frameSize, image_width, image_height);
        CvSeq *faces = cvHaarDetectObjects(
                                           img,
                                           cascade,
                                           storage,
                                           1.1,
                                           3,
                                           0 /*CV_HAAR_DO_CANNY_PRUNNING*/,
                                           cvSize( 40, 40 ) );
        
               /* for each face found, draw a red box */
        if(faces->total !=0) {
            CvRect *r = ( CvRect* )cvGetSeqElem( faces, 0 );
            printf("%d, %d, %d, %d\n",r->x, r->y, r->width, r->height);
        
            bbox[0] = r->x;
            bbox[1] = r->y;
            bbox[2] = r->width;
            bbox[3] = r->height;
            double screen[4];
            bboxToScreen(bbox,screen);
            printf("%lf, %lf, %lf, %lf\n",screen[0],screen[1],screen[2],screen[3]);
            [[NSInvocation invocationWithTarget:self.rect.layer selector:@selector(setFrame:) retainArguments:NO, CGRectMake(screen[0],screen[1],screen[2],screen[3])] invokeOnMainThreadWaitUntilDone:NO];

        

        }
    } else  if (is_initialized) {	
        
        unsigned char *pixelData = [lastImage grayscalePixels];
        
        unsigned char* frame = (unsigned char*) malloc (sizeof(unsigned char) * image_width * image_height);
        for (int i = 0; i < image_height; i++) {
            for (int j = 0; j < image_width; j++) {
                //90 degree rotation
                frame[j*(int)image_width + ((int)image_height-i-1)] = pixelData[i*(int)image_width+j];
            }
        }
        [[NSInvocation invocationWithTarget:self.prevLayer selector:@selector(setHidden:) retainArguments:NO, YES] invokeOnMainThreadWaitUntilDone:NO];
		tld->tldProcessFrame(image_width, image_height, frame, bbox, output_bbox, MIN_TRACKING_CONF, MIN_REINIT_CONF, MIN_LEARNING_CONF);
        
        
        bbox[0] = output_bbox[0];
        bbox[1] = output_bbox[1];
        bbox[2] = output_bbox[2];
        bbox[3] = output_bbox[3];
        if(output_bbox[2] > 0 && output_bbox[3] > 0) { 
            
            bboxToScreen(bbox,screen);
            [[NSInvocation invocationWithTarget:self.rect.layer selector:@selector(setHidden:) retainArguments:NO, NO] invokeOnMainThreadWaitUntilDone:NO];
            [[NSInvocation invocationWithTarget:self.rect.layer selector:@selector(setFrame:) retainArguments:NO, CGRectMake(screen[0],screen[1],screen[2],screen[3])] invokeOnMainThreadWaitUntilDone:NO];
        } else {
            [[NSInvocation invocationWithTarget:self.rect.layer selector:@selector(setHidden:) retainArguments:NO, YES] invokeOnMainThreadWaitUntilDone:NO];
        }
    }
    
		
	//release the source
	free(baseAddressGray);
	CGColorSpaceRelease(colorSpaceGray);
    // CGColorSpaceRelease(colored);
	CGContextRelease(newContextGray);
    
	/*We unlock the  image buffer*/
	CVPixelBufferUnlockBaseAddress(imageBuffer,0);
	
	[pool drain];
	
	fps += 1;
} 

-(IBAction) handleGesture: (UIGestureRecognizer *) sender {
    static CGPoint starting;
    static BOOL started = NO;
    
    
    CGPoint translation = [(UIPanGestureRecognizer *)sender translationInView:self.imageView];
    if(sender.state == UIGestureRecognizerStateBegan) {
        starting =  [(UIPanGestureRecognizer *)sender locationInView:self.imageView];
        started = YES;
        self.rect.hidden = NO;
        if(tld != 0) {
            free(tld);
            tld  = 0;
            is_initialized = false;
        }
        return;
        
    }
    if(sender.state == UIGestureRecognizerStateEnded) {
        started = NO;
        float width = 360;
        float height = 480; 
        image_height = lastImage.size.height;
        image_width = lastImage.size.width;
        float x0 = starting.x;
        float y0 = starting.y;
        float dx = translation.x;
        float dy = translation.y;
        if(dx < 0 ) {
            x0+=dx;
            dx*=-1;
        }
        if( dy < 0 ) { 
            y0+=dy;
            dy*=-1;
        }
        float screen_x = y0*lastImage.size.height/height;
        float screen_y = lastImage.size.width-lastImage.size.width*(x0+dx)/width;
        float screen_dx = dy*lastImage.size.height/height;
        float screen_dy = dx/width*lastImage.size.width;
        
        CGRect fromRect = CGRectMake(screen_x,screen_y,screen_dx,screen_dy);
        
        CGImageRef drawImage = CGImageCreateWithImageInRect(lastImage.CGImage, fromRect);
        UIImage *newImage = [UIImage imageWithCGImage:drawImage scale:1.0 orientation:UIImageOrientationRight];
        CGImageRelease(drawImage);
        self.prevLayer.image = newImage;
        
        unsigned char *pixelData = [lastImage grayscalePixels];
        
        unsigned char* frame = (unsigned char*) malloc (sizeof(unsigned char) * image_width * image_height);
        for (int i = 0; i < image_height; i++) {
            for (int j = 0; j < image_width; j++) {
                //90 degree rotation
                frame[j*(int)image_width + ((int)image_height-i-1)] = pixelData[i*(int)image_width+j];
            }
        }
        
        //struct size frame_size;
        
        if (!is_initialized) {
            screen[0] = x0;
            screen[1] = y0;
            screen[2] = dx;
            screen[3] = dy;
            screenToBbox(screen,bbox);
            output_bbox[0] = 0; output_bbox[1] = 0; output_bbox[2] = 0; output_bbox[3] = 0;
            printf("First BB: (%lf, %lf, %lf, %lf)\n",bbox[0], bbox[1], bbox[2], bbox[3]);
            tld = new Tld((int)image_width, (int) image_height, frame, bbox, nTREES, nFEAT,  MIN_FEATURE_SCALE,  MAX_FEATURE_SCALE,  WIDTH_STEPS,  HEIGHT_STEPS) ;
          is_initialized = true;
           // use_face_detection = true;
        }
        
        
    } //if we are still drawing: 
    self.rect.layer.borderColor = [UIColor redColor].CGColor;
    self.rect.layer.borderWidth = 3.0f;
    self.rect.frame = self.view.frame;
    self.rect.layer.frame = CGRectMake(starting.x,starting.y,translation.x,translation.y);
    
}


- (IBAction) btnclicked:(id) sender {
    
    [self.captureSession stopRunning];
    delete tld;
    is_initialized = false;
    tld = 0;

    
}
-(void) viewWillDisappear:(BOOL)animated {
    if ([self.navigationController.viewControllers indexOfObject:self]==NSNotFound) {
        [self.captureSession stopRunning];
        delete tld;
        is_initialized = false;
        tld = 0;
    }
    [super viewWillDisappear:animated];
}
- (IBAction) openAdvanceSettings: (id) sender {
    myAdvanceCameraViewController  = [[AdvanceCameraViewController alloc] initWithNibName:nil bundle:nil];
    [self.navigationController pushViewController:myAdvanceCameraViewController animated:YES];
    
}
+ (void) toggleFps {
    if(show_fps) {
        show_fps = false;
    } else {
        show_fps = true;
    }
}
+ (void) setMinLearn: (float) value {
    MIN_LEARNING_CONF = value;
}
+ (void) setMinTrack: (float) value {
    MIN_TRACKING_CONF = value;
}
+ (void) setMinScale: (float) value {
    return;
}
+ (void) setMaxScale: (float) value {
    return;
}
+ (void) setWidthSteps: (int) value {
    WIDTH_STEPS = value;
}
+ (void) setHeightSteps: (int) value {
    HEIGHT_STEPS = value;
}
+ (void) setNumFeatures: (int) value {
    nFEAT = value;
}
+ (void) setNumTrees: (int) value {
    nTREES = value;
}

@end