//
//  PredatorViewController.m
//  Predator
//
//  Created by admin on 10/25/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "PredatorViewController.h"
#import "CameraViewController.h"
#import "VideoViewController.h"
#import "AboutViewController.h"




@implementation PredatorViewController
 CameraViewController* myCameraViewController;
 VideoViewController* myVideoViewController;
 AboutViewController* myAboutViewController;

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


// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
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
- (IBAction) cameraClick:(id)sender {
    myCameraViewController = [[CameraViewController alloc] initWithNibName:nil bundle:nil];
    [self.navigationController pushViewController:myCameraViewController animated:YES];

}
- (IBAction) videoClick:(id)sender {
    myVideoViewController = [[VideoViewController alloc] initWithNibName:nil bundle:nil];
    [self.navigationController pushViewController:myVideoViewController animated:YES];



}
- (IBAction) aboutClick:(id)sender {
    myAboutViewController  = [[AboutViewController alloc] initWithNibName:nil bundle:nil];
    [self.navigationController pushViewController:myAboutViewController animated:YES];

}
- (void) viewWillAppear:(BOOL)animated
{
    [self.navigationController setNavigationBarHidden:YES animated:animated];
    [super viewWillAppear:animated];
}

- (void) viewWillDisappear:(BOOL)animated
{
    [self.navigationController setNavigationBarHidden:NO animated:animated];
    [super viewWillDisappear:animated];
}

@end
