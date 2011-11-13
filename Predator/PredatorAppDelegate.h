//
//  PredatorAppDelegate.h
//  Predator
//
//  Created by admin on 10/25/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

//@class PredatorViewController;

@interface PredatorAppDelegate : NSObject <UIApplicationDelegate> {

}

@property (nonatomic, retain) IBOutlet UIWindow *window;

//@property (nonatomic, retain) IBOutlet PredatorViewController *viewController;
@property (nonatomic, retain) IBOutlet UINavigationController *navigationController;

@end
