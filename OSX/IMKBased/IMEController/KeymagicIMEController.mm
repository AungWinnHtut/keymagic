//
// KeymagicIMEController.m
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

#import <Carbon/Carbon.h>
#import <Foundation/Foundation.h>

#import "KeymagicIMEController.h"
#import	"keymagic.h"
#import "KeyMagicUtil.h"

#define kLastKeyboardPathKey @"DefaultKeyboardPath"
#define kInstantCommit @"InstantCommit"

@interface KeyMagicIMEController () <NSUserNotificationCenterDelegate>

@property (nonatomic, strong) NSMutableArray *keyboards;

@end


@implementation KeyMagicIMEController
@synthesize activeKeyboard;
@synthesize activePath;
@synthesize keyboards;
@synthesize instantCommit;

#define RETURNVAL(b, c) \
case b: \
*winVK = c;\
return TRUE

// Map OS X Virtual Key to Windows Virtual Key
bool mapVK(int virtualkey, int * winVK)
{
	switch (virtualkey) {
		RETURNVAL(kVK_ANSI_A, 'A');
		RETURNVAL(kVK_ANSI_B, 'B');
		RETURNVAL(kVK_ANSI_C, 'C');
		RETURNVAL(kVK_ANSI_D, 'D');
		RETURNVAL(kVK_ANSI_E, 'E');
		RETURNVAL(kVK_ANSI_F, 'F');
		RETURNVAL(kVK_ANSI_G, 'G');
		RETURNVAL(kVK_ANSI_H, 'H');
		RETURNVAL(kVK_ANSI_I, 'I');
		RETURNVAL(kVK_ANSI_J, 'J');
		RETURNVAL(kVK_ANSI_K, 'K');
		RETURNVAL(kVK_ANSI_L, 'L');
		RETURNVAL(kVK_ANSI_M, 'M');
		RETURNVAL(kVK_ANSI_N, 'N');
		RETURNVAL(kVK_ANSI_O, 'O');
		RETURNVAL(kVK_ANSI_P, 'P');
		RETURNVAL(kVK_ANSI_Q, 'Q');
		RETURNVAL(kVK_ANSI_R, 'R');
		RETURNVAL(kVK_ANSI_S, 'S');
		RETURNVAL(kVK_ANSI_T, 'T');
		RETURNVAL(kVK_ANSI_U, 'U');
		RETURNVAL(kVK_ANSI_V, 'V');
		RETURNVAL(kVK_ANSI_W, 'W');
		RETURNVAL(kVK_ANSI_X, 'X');
		RETURNVAL(kVK_ANSI_Y, 'Y');
		RETURNVAL(kVK_ANSI_Z, 'Z');
		RETURNVAL(kVK_ANSI_0, '0');
		RETURNVAL(kVK_ANSI_1, '1');
		RETURNVAL(kVK_ANSI_2, '2');
		RETURNVAL(kVK_ANSI_3, '3');
		RETURNVAL(kVK_ANSI_4, '4');
		RETURNVAL(kVK_ANSI_5, '5');
		RETURNVAL(kVK_ANSI_6, '6');
		RETURNVAL(kVK_ANSI_7, '7');
		RETURNVAL(kVK_ANSI_8, '8');
		RETURNVAL(kVK_ANSI_9, '9');
		RETURNVAL(kVK_ANSI_Equal, VK_OEM_PLUS);
		RETURNVAL(kVK_ANSI_Minus, VK_OEM_MINUS);
		RETURNVAL(kVK_ANSI_LeftBracket, VK_OEM_4);
		RETURNVAL(kVK_ANSI_RightBracket, VK_OEM_6);
		RETURNVAL(kVK_ANSI_Quote, VK_OEM_7);
		RETURNVAL(kVK_ANSI_Semicolon, VK_OEM_1);
		RETURNVAL(kVK_ANSI_Backslash, VK_OEM_5);
		RETURNVAL(kVK_ANSI_Comma, VK_OEM_COMMA);
		RETURNVAL(kVK_ANSI_Slash, VK_OEM_2);
		RETURNVAL(kVK_ANSI_Period, VK_OEM_PERIOD);
		RETURNVAL(kVK_ANSI_Grave, VK_OEM_3);
		RETURNVAL(kVK_ANSI_KeypadDecimal, VK_DECIMAL);
		RETURNVAL(kVK_ANSI_KeypadMultiply, VK_MULTIPLY);
		RETURNVAL(kVK_ANSI_KeypadPlus, VK_ADD);
		RETURNVAL(kVK_ANSI_KeypadDivide, VK_DIVIDE);
		RETURNVAL(kVK_ANSI_KeypadMinus, VK_SUBTRACT);
		RETURNVAL(kVK_ANSI_Keypad0, VK_NUMPAD0);
		RETURNVAL(kVK_ANSI_Keypad1, VK_NUMPAD1);
		RETURNVAL(kVK_ANSI_Keypad2, VK_NUMPAD2);
		RETURNVAL(kVK_ANSI_Keypad3, VK_NUMPAD3);
		RETURNVAL(kVK_ANSI_Keypad4, VK_NUMPAD4);
		RETURNVAL(kVK_ANSI_Keypad5, VK_NUMPAD5);
		RETURNVAL(kVK_ANSI_Keypad6, VK_NUMPAD6);
		RETURNVAL(kVK_ANSI_Keypad7, VK_NUMPAD7);
		RETURNVAL(kVK_ANSI_Keypad8, VK_NUMPAD8);
		RETURNVAL(kVK_ANSI_Keypad9, VK_NUMPAD9);
		RETURNVAL(kVK_Return, VK_RETURN);
		RETURNVAL(kVK_Tab, VK_TAB);
		RETURNVAL(kVK_Space, VK_SPACE);
		RETURNVAL(kVK_Delete, VK_BACK);
		RETURNVAL(kVK_ForwardDelete, VK_DELETE);
		RETURNVAL(kVK_Escape, VK_ESCAPE);
		RETURNVAL(kVK_Shift, VK_SHIFT);
		RETURNVAL(kVK_CapsLock, VK_CAPITAL);
		RETURNVAL(kVK_Option, VK_MENU);
		RETURNVAL(kVK_Control, VK_CONTROL);
		RETURNVAL(kVK_RightShift, VK_RSHIFT);
		RETURNVAL(kVK_RightOption, VK_RMENU);
		RETURNVAL(kVK_RightControl, VK_RCONTROL);
		default:
			break;
	}
	return FALSE;
}

#undef RETURNVAL

- (id)initWithServer:(IMKServer*)server delegate:(id)delegate client:(id)inputClient
{
    self = [super initWithServer:server delegate:delegate client:inputClient];
    
    if (self) {
        [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:self];
        
        self.activeKeyboard = [[Keyboard alloc] init];
        self.keyboards = [[NSMutableArray alloc] init];
        
        char logPath[300];
        char * home = getenv("HOME");
        sprintf(logPath, "%s%s", home, "/Library/Logs/KeyMagic.log");
        m_logFile = fopen(logPath, "w");
        
        logger = KeyMagicLogger::getInstance();
        if (m_logFile != 0) logger->setFile(m_logFile);
        kme.m_verbose = true;
        
        [self getKeyboardLayouts];
        
        if ([super initWithServer:server delegate:delegate client:inputClient] == self) {	
            configDictionary = [NSMutableDictionary new];
            self.activePath = @"";
            
            [self loadConfigurationFile];
            
            instantCommit = [[configDictionary objectForKey:kInstantCommit] boolValue];
            
            m_success = NO;
            m_delCountGenerated = 0;
            NSString *path = [configDictionary objectForKey:kLastKeyboardPathKey];
            
            if (path) {
                self.activePath = path;
                m_success = kme.loadKeyboardFile([activePath cStringUsingEncoding:NSUTF8StringEncoding]);
                if (m_success) {
                    const InfoList infos = kme.getKeyboard()->getInfoList();
                    NSString *title = [KeyMagicUtil getKeyboardNameOrTitle:infos pathName:path];
                                
                    [activeKeyboard setTitle:title];
                    [activeKeyboard setPath:path];
                    
                    NSUserNotification *notification = [[NSUserNotification alloc] init];
                    notification.title = @"KeyMagic";
                    notification.informativeText = activeKeyboard.title;
                    notification.hasActionButton = NO;
                    [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:notification];;
                }
            }
        }
    }
	
	return self;
}


- (void)activateServer:(id)sender
{
}

- (void)deactivateServer:(id)sender
{
}

- (BOOL)isTSMDocumentAccessSupportedClient:(id)sender {
	NSRange range = [sender selectedRange];
	if (range.location == NSNotFound || range.length == NSNotFound) {
		NSLog(@"NO TSM ACCESS %@", NSStringFromRange(range));
		return NO;
	} else {
		NSLog(@"YES TSM ACCESS %@", NSStringFromRange(range));
		return YES;
	}
}

- (void)commitComposition:(id)sender 
{
	if (instantCommit) {
		kme.reset();
	} else {
		NSString * _composingBuffer = [NSString stringWithKeyMagicString:kme.getContextText()];
		if ([_composingBuffer length]) {
			[sender insertText:_composingBuffer replacementRange:NSMakeRange(NSNotFound, NSNotFound)];
			kme.reset();
		}
	}
}

- (void)switchKeyboardLayout:(BOOL)previous
{
	Keyboard * first = [keyboards objectAtIndex:0];
	Keyboard * last = [keyboards objectAtIndex:[keyboards count] - 1];
	
	NSEnumerator * e;
	if (previous) {
		e = [keyboards reverseObjectEnumerator];
	} else {
		e = [keyboards objectEnumerator];
	}
	
	while (Keyboard * kb = [e nextObject]) {
		if ([kb.path compare:activePath] == NSOrderedSame) {
			if (kb = [e nextObject]) {
				[self changeKeyboardLayout:kb];
			} else {
				if (previous) {
					[self changeKeyboardLayout:last];
				} else {
					[self changeKeyboardLayout:first];
				}
			}
		}
	}
}

- (BOOL)processSpecialKeys:(NSEvent*)event client:(id)sender exit:(BOOL *)exit {
	
//	NSString *chars = [event characters];
	unsigned int cocoaModifiers = [event modifierFlags];
	unsigned short virtualKeyCode = [event keyCode];
	
	if (virtualKeyCode == kVK_Space && (cocoaModifiers & NSControlKeyMask) && (cocoaModifiers & NSShiftKeyMask)) {
		[self switchKeyboardLayout:YES]; // <-
		*exit = YES;
		return YES;
	} else if (virtualKeyCode == kVK_Space && (cocoaModifiers & NSControlKeyMask)) {
		[self switchKeyboardLayout:NO]; // ->
		*exit = YES;
		return YES;
	}
	
	switch (virtualKeyCode) {
		case kVK_LeftArrow:
		case kVK_UpArrow:
		case kVK_RightArrow:
		case kVK_DownArrow:
			
		case kVK_Home:
		case kVK_End:
			
		case kVK_PageUp:
		case kVK_PageDown:
			[self commitComposition:sender];
			*exit = YES;
			return NO;
	}
	
	int winVK;
	if (mapVK(virtualKeyCode, &winVK) == NO) {
		*exit = YES;
		return NO;
	}
	
	*exit = NO;
	return NO;
}

- (void)printEngineHistory:(const TContextHistory &)history {
	TContextHistory::const_iterator begin = history.begin();
	for (TContextHistory::const_iterator i = begin; i != history.end(); i++) {
		NSLog(@"%ld- %@", i - begin, [NSString stringWithKeyMagicString:*i]);
	}
}

- (BOOL)engineProcessWithEvent:(NSEvent *)event client:(id)sender exit:(BOOL *)exit {
	
	NSString *chars = [event characters];
	unsigned int cocoaModifiers = [event modifierFlags];
	unsigned short virtualKeyCode = [event keyCode];
	
	unsigned char kbStates[256] = {0};
	
	int modifier = 0;
	if (cocoaModifiers & NSCommandKeyMask) {
		*exit = YES;
		return NO;
	}
	if (cocoaModifiers & NSAlphaShiftKeyMask) {
		modifier |= KeyMagicEngine::CAPS_MASK;
		kbStates[VK_CAPITAL] = 0x81;
	}
	if (cocoaModifiers & NSShiftKeyMask) {
		modifier |= KeyMagicEngine::SHIFT_MASK;
		kbStates[VK_SHIFT] = 0x80;
	}
	if (cocoaModifiers & NSControlKeyMask) {
		modifier |= KeyMagicEngine::CTRL_MASK;
		kbStates[VK_CONTROL] = 0x80;
	}
	if (cocoaModifiers & NSAlternateKeyMask) {
		modifier |= KeyMagicEngine::ALT_MASK;
		kbStates[VK_MENU] = 0x80;
	}
	
	int winVK;
	if (mapVK(virtualKeyCode, &winVK) == NO) {
		*exit = YES;
		return NO;
	}
	
	kme.setKeyStates(kbStates);
	NSLog(@"processKeyEvent = %c 0x%x 0x%x", [chars characterAtIndex:0], winVK, modifier);
	if (kme.processKeyEvent([chars characterAtIndex:0], winVK, modifier) == NO) {
		switch (virtualKeyCode) {
			case kVK_Space:
			case kVK_Return:
				[self commitComposition:sender];
				break;
		}
		
		if (cocoaModifiers & NSControlKeyMask) {
			[self commitComposition:sender];
		} else if (cocoaModifiers & NSAlternateKeyMask) {
			[self commitComposition:sender];
		} else if (cocoaModifiers & NSCommandKeyMask) {
			[self commitComposition:sender];
		}
		*exit = YES;
		return NO;
	} else {
		[self printEngineHistory:kme.getHistory()];
	}
	
	*exit = NO;
	return NO;
}

- (BOOL)handleEvent:(NSEvent*)event TSMDocumentAccessSupportedClient:(id)sender
{
    if ([event type] != NSKeyDown || m_success == NO) {
		return NO;
	}
	
	if (m_delCountGenerated && [event keyCode] == kVK_Delete) {
		m_delCountGenerated--;
		return NO;
	}
	
	NSRange selRange = [self.client selectedRange];
	NSUInteger lengthToRetrive = selRange.location;
	
	if (lengthToRetrive > 10) {
		lengthToRetrive = 10;
	}
	
	NSAttributedString *beforeCursorContext = [self.client attributedSubstringFromRange:NSMakeRange(selRange.location - lengthToRetrive, lengthToRetrive)];
	if (!beforeCursorContext) {
		beforeCursorContext = [[NSAttributedString alloc] initWithString:@""];
	}
	
	BOOL exit, result;
	result = [self processSpecialKeys:event client:sender exit:&exit];
	if (exit) return result;
	
	NSString * memoryContext = [NSString stringWithKeyMagicString:kme.getContextText() maximumLength:beforeCursorContext.length fromEnd:YES];
	
    if (![beforeCursorContext.string isEqualToString:memoryContext]) {
		NSLog(@"diff %@ != %@", beforeCursorContext, memoryContext);
		kme.reset();
		kme.setContextText([beforeCursorContext.string getKeyMagicString]);
	} else {
		NSLog(@"same %@", memoryContext);
	}
	
	KeyMagicString beforeProcessed = kme.getContextText();
	
	result = [self engineProcessWithEvent:event client:sender exit:&exit];
	if (exit) return result;
	
	KeyMagicString afterProcessed = kme.getContextText();
	unsigned int delCount = 0;
	
	KeyMagicString *output = new KeyMagicString();
	getDifference(beforeProcessed, afterProcessed, &delCount, output);
	
	m_delCountGenerated = 0;
	
	if (delCount || output->length()) {
		NSString *textToInsert = [NSString stringWithKeyMagicString:output->c_str()];
		NSInteger replacementLocation = selRange.location - delCount;
		
		if (replacementLocation < 0) {
			replacementLocation = 0;
		}
		
		NSRange replacementRange = NSMakeRange(replacementLocation, selRange.length + delCount);
		
		NSAttributedString *attributedTextToInsert = [[NSAttributedString alloc] initWithString:textToInsert
																		   attributes:[NSDictionary
																					   dictionaryWithObject:NSStringFromRange(replacementRange)
																					   forKey:NSTextInputReplacementRangeAttributeName]];
		
		NSLog(@"text = %@, delCount = %d, replacementRange = %@", textToInsert, delCount, NSStringFromRange(replacementRange));
		
		if (textToInsert.length == 0 && delCount) {
			
			CGEventRef down, up, uni;
			down = CGEventCreateKeyboardEvent (NULL, (CGKeyCode)51, true);
			up = CGEventCreateKeyboardEvent (NULL, (CGKeyCode)51, false);
//		uni = CGEventCreateKeyboardEvent (NULL, (CGKeyCode)1, true);
			
			m_delCountGenerated = delCount;
			
			while (delCount--) {
				CGEventPost(kCGSessionEventTap, down);
				CGEventPost(kCGSessionEventTap, up);
			}
            
            CFRelease(down);
            CFRelease(up);
		
		} else {
			[sender insertText:attributedTextToInsert replacementRange:replacementRange];
		}
	}
	delete output;
    
	return YES;
}

- (BOOL)handleEvent:(NSEvent*)event TSMDocumentAccessUnSupportedClient:(id)sender
{
    if ([event type] != NSKeyDown || m_success == NO) {
		return NO;
	}
	
	BOOL exit, result;
	result = [self processSpecialKeys:event client:sender exit:&exit];
	if (exit) return result;
	
	result = [self engineProcessWithEvent:event client:sender exit:&exit];
	if (exit) return result;
	
	NSString * _composingBuffer = [NSString stringWithKeyMagicString:kme.getContextText()];
	NSMutableAttributedString *attrString = [[NSMutableAttributedString alloc] initWithString:_composingBuffer attributes:[NSDictionary dictionary]];

    #if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5
		NSDictionary *attrDict = [NSDictionary dictionaryWithObjectsAndKeys:
								  [NSNumber numberWithInt:0], NSKernAttributeName,
								  [NSNumber numberWithInt:NSUnderlineStyleSingle], NSUnderlineStyleAttributeName,
								  [NSNumber numberWithInt:0], NSMarkedClauseSegmentAttributeName, nil];
    #else
		NSDictionary *attrDict = [NSDictionary dictionaryWithObjectsAndKeys:
								  [NSNumber numberWithInt:NSUnderlineStyleSingle], @"UnderlineStyleAttribute",
								  [NSNumber numberWithInt:0], @"MarkedClauseSegmentAttribute", nil];
    #endif

	[attrString setAttributes:attrDict range:NSMakeRange(0, [_composingBuffer length])];  
	
	// selectionRange means "cursor position index"
	NSRange selectionRange = NSMakeRange([_composingBuffer length], 0); 
	[sender setMarkedText:attrString selectionRange:selectionRange replacementRange:NSMakeRange(NSNotFound, NSNotFound)];
	
    return YES;
}

- (BOOL)handleEvent:(NSEvent*)event client:(id)sender
{
	if (instantCommit) {
		return [self handleEvent:event TSMDocumentAccessSupportedClient:sender];
	} else {
		return [self handleEvent:event TSMDocumentAccessUnSupportedClient:sender];
	}
}

- (void)_aboutAction:(id)sender
{
	[NSApp orderFrontStandardAboutPanel:sender];
}

- (void)instantCommitMenuClicked:(id)sender
{
	BOOL instant = ![[configDictionary objectForKey:kInstantCommit] boolValue];
	[configDictionary setObject:[NSNumber numberWithBool:instant] forKey:kInstantCommit];
	instantCommit = instant;
	kme.reset();
	
	[self writeConfigurationFile];
}

- (BOOL)changeKeyboardLayout:(Keyboard*)keyboard
{
	if (keyboard.path != nil) {
		if ((m_success = kme.loadKeyboardFile([keyboard.path cStringUsingEncoding:NSUTF8StringEncoding]))) {
			[configDictionary setObject:[keyboard path] forKey:kLastKeyboardPathKey];
			self.activePath = [keyboard path];
			self.activeKeyboard = keyboard;
			[self writeConfigurationFile];
			
			@try {
                NSUserNotification *notification = [[NSUserNotification alloc] init];
                notification.title = @"KeyMagic";
                notification.informativeText = keyboard.title;
                notification.hasActionButton = NO;
                [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:notification];;

			}
			@catch (NSException * e) {
				NSLog(@"Failed to notify with Growl!");
			}
		}
	}
}

- (void)selectionChanged:(id)sender {
	NSMenuItem *menuItem = [sender objectForKey:@"IMKCommandMenuItem"];
	Keyboard * keyboard = [menuItem representedObject];
	[self changeKeyboardLayout:keyboard];
}

- (NSArray *)getKeyboardPathsFrom:(NSString*)directory
{
	NSMutableArray * paths = [NSMutableArray new];
	NSFileManager *localFileManager = [[NSFileManager alloc] init];
	NSDirectoryEnumerator *dirEnum = [localFileManager enumeratorAtPath:directory];
	
	NSString *file;
	while (file = [dirEnum nextObject]) {
		if ([[file pathExtension] isEqualToString: @"km2"]) {
			[paths addObject:[directory stringByAppendingPathComponent:file]];
		}
	}
	
	return paths;
}

- (void)getKeyboardLayouts
{
	[keyboards removeAllObjects];
	
	NSMutableArray * allPaths = [NSMutableArray new];
	NSArray * paths;
	
	NSString *layoutDir = [NSHomeDirectory() stringByAppendingPathComponent:@".keymagic"];
	paths = [self getKeyboardPathsFrom:layoutDir];
	[allPaths addObjectsFromArray:paths];
	
	NSBundle * mainBundle = [NSBundle mainBundle];
	paths = [mainBundle pathsForResourcesOfType:@"km2" inDirectory:nil];
	[allPaths addObjectsFromArray:paths];

	NSEnumerator *e = [allPaths objectEnumerator];
	
	while (NSString *path = [e nextObject]) {		
		const char * szPath = [path cStringUsingEncoding:NSUTF8StringEncoding];
		InfoList * infos = KeyMagicKeyboard::getInfosFromKeyboardFile(szPath);
		
		if (infos == NULL) {
			continue;
		}
		
//		NSMenuItem * menuItem = [NSMenuItem new];
		NSString * keyboardName = [KeyMagicUtil getKeyboardNameOrTitle:*infos pathName:path];

		Keyboard * keyboard = [Keyboard new];
		[keyboard setTitle:keyboardName];
		[keyboard setPath:path];
		
		[keyboards addObject:keyboard];
		
//		for (InfoList::iterator i = infos->begin(); i != infos->end(); i++) {
//			delete[] i->second.data;
//		}
		delete infos;
	}
}

- (NSMenu *)menu
{
	NSMenu *menu = [NSMenu new];
	
	[self getKeyboardLayouts];
	NSEnumerator * e = [keyboards objectEnumerator];
	while (Keyboard * kb = [e nextObject]) {
		NSMenuItem * menuItem = [[NSMenuItem alloc] init];
		
		[menuItem setTarget:self];
		[menuItem setTitle:kb.title];
		[menuItem setAction:@selector(selectionChanged:)];
		[menuItem setRepresentedObject:kb];
		
		if ([kb.path compare:activePath] == NSOrderedSame) {
			[menuItem setState:NSOnState];
		}
		
		[menu addItem:menuItem];
	}
	
	[menu addItem:[NSMenuItem separatorItem]];
	
	NSMenuItem *menuItem;
    menuItem = [NSMenuItem new];
    [menuItem setTarget:self];
    [menuItem setAction:@selector(instantCommitMenuClicked:)];
    [menuItem setTitle:@"Instant Commit"];
    if ([[configDictionary objectForKey:kInstantCommit] boolValue] == YES) {
        [menuItem setState:NSOnState];
    }
    [menu addItem:menuItem];
    
    [menu addItem:[NSMenuItem separatorItem]];
    
    menuItem = [NSMenuItem new];
	[menuItem setTarget:self];
	[menuItem setAction:@selector(_aboutAction:)];
	[menuItem setTitle:@"About KeyMagic"];
	[menu addItem:menuItem];
	return menu;
}

- (NSString*)configFilePath {
	NSArray *dirs = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
	NSString *prefPath = [[dirs objectAtIndex:0] stringByAppendingPathComponent:@"Preferences"];	
	return [prefPath stringByAppendingPathComponent:@"org.keymagic.plist"];
}

- (void)writeConfigurationFile {
	NSData *data = [NSPropertyListSerialization dataFromPropertyList:configDictionary format:NSPropertyListXMLFormat_v1_0 errorDescription:nil];
	if (data) {
		[data writeToFile:[self configFilePath] atomically:YES];
	}
}

- (void)loadConfigurationFile {
	NSData *data = [NSData dataWithContentsOfFile:[self configFilePath]];
	if (data) {
		NSPropertyListFormat format;
		id plist = [NSPropertyListSerialization propertyListFromData:data mutabilityOption:NSPropertyListMutableContainersAndLeaves format:&format errorDescription:nil];
		if ([plist isKindOfClass:[NSDictionary class]]) {
			[configDictionary removeAllObjects];
			[configDictionary addEntriesFromDictionary:plist];
		}		
	}
}

- (void)userNotificationCenter:(NSUserNotificationCenter *)center didDeliverNotification:(NSUserNotification *)notification
{
    [center performSelector:@selector(removeDeliveredNotification:) withObject:notification afterDelay:1.5];
}

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification
{
    return YES;
}

@end
