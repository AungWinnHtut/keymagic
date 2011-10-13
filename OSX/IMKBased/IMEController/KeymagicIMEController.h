//
// KeymagicIMEController.h
//
//Copyright (C) 2008  KeyMagic Project
//http://keymagic.googlecode.com
//
//This program is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#import <Cocoa/Cocoa.h>
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5
 	#import <InputMethodKit/InputMethodKit.h>
#else
	#import "InputMethodKitTiger.h"
#endif
#import "KeyMagicNSString.h"
#import "Keyboard.h"

#include "KeyMagicEngine.h"

extern "C" {
	extern NSString *NSTextInputReplacementRangeAttributeName;
}

@interface KeyMagicIMEController : IMKInputController
{
	NSMutableArray * keyboards;
	NSMutableDictionary * configDictionary;
	NSString *activePath;
	KeyMagicEngine kme;
	BOOL m_success;
	FILE * m_logFile;
	KeyMagicLogger * logger;
	NSStatusItem * statusItem;
	Keyboard * activeKeyboard;
	NSUInteger m_delCountGenerated;
	BOOL instantCommit;
}

@property (nonatomic, retain) Keyboard *activeKeyboard;
@property (nonatomic, copy) NSString *activePath;
@property (nonatomic, assign) BOOL instantCommit;

- (void)selectionChanged:(id)sender;
- (void)WriteConfigurationFile;
- (void)LoadConfigurationFile;
- (NSArray *)getKeyboardPathsFrom:(NSString*)directory;
- (BOOL)changeKeyboardLayout:(Keyboard*)Keyboard;
- (void)switchKeyboardLayout:(BOOL)previous;
- (void)GetKeyboardLayouts;

@end
