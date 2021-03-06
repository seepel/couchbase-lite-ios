//
//  CBLChangeTracker.h
//  CouchbaseLite
//
//  Created by Jens Alfke on 6/20/11.
//  Copyright 2011 Couchbase, Inc.
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
//  except in compliance with the License. You may obtain a copy of the License at
//    http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software distributed under the
//  License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
//  either express or implied. See the License for the specific language governing permissions
//  and limitations under the License.

#import <Foundation/Foundation.h>
@class CBLChangeTracker;
@protocol CBLAuthorizer;


@protocol CBLChangeTrackerClient <NSObject>
- (BOOL) changeTrackerApproveSSLTrust: (SecTrustRef)serverTrust
                              forHost: (NSString*)host
                                 port: (UInt16)port;
@optional
- (void) changeTrackerReceivedChange: (NSDictionary*)change;
- (void) changeTrackerReceivedChanges: (NSArray*)changes;
- (void) changeTrackerStopped: (CBLChangeTracker*)tracker;
@end


typedef enum CBLChangeTrackerMode {
    kOneShot,
    kLongPoll,
    kContinuous
} CBLChangeTrackerMode;


/** Reads the continuous-mode _changes feed of a database, and sends the individual change entries to its client.  */
@interface CBLChangeTracker : NSObject <NSStreamDelegate>
{
    @protected
    NSURL* _databaseURL;
    id<CBLChangeTrackerClient> __weak _client;
    CBLChangeTrackerMode _mode;
    id _lastSequenceID;
    unsigned _limit;
    NSError* _error;
    BOOL _continuous;
    BOOL _includeConflicts;
    NSString* _filterName;
    NSDictionary* _filterParameters;
    NSTimeInterval _heartbeat;
    NSDictionary* _requestHeaders;
    id<CBLAuthorizer> _authorizer;
    unsigned _retryCount;
}

- (instancetype) initWithDatabaseURL: (NSURL*)databaseURL
                                mode: (CBLChangeTrackerMode)mode
                           conflicts: (BOOL)includeConflicts
                        lastSequence: (id)lastSequenceID
                              client: (id<CBLChangeTrackerClient>)client;

@property (readonly, nonatomic) NSURL* databaseURL;
@property (readonly, nonatomic) NSString* databaseName;
@property (readonly) NSURL* changesFeedURL;
@property (readonly, copy, nonatomic) id lastSequenceID;
@property (nonatomic) BOOL continuous;  // If true, never give up due to errors
@property (strong, nonatomic) NSError* error;
@property (weak, nonatomic) id<CBLChangeTrackerClient> client;
@property (strong, nonatomic) NSDictionary *requestHeaders;
@property (strong, nonatomic) id<CBLAuthorizer> authorizer;

@property (nonatomic) CBLChangeTrackerMode mode;
@property (copy) NSString* filterName;
@property (copy) NSDictionary* filterParameters;
@property (nonatomic) unsigned limit;
@property (nonatomic) NSTimeInterval heartbeat;
@property (nonatomic) NSArray *docIDs;

- (BOOL) start;
- (void) stop;

/** Asks the tracker to retry connecting, _if_ it's currently disconnected but waiting to retry.
    This should be called when the reachability of the remote host changes, or when the
    app is reactivated. */
- (void) retry;

// Protected
@property (readonly) NSString* changesFeedPath;
- (void) setUpstreamError: (NSString*)message;
- (void) failedWithError: (NSError*)error;
- (NSInteger) receivedPollResponse: (NSData*)body errorMessage: (NSString**)errorMessage;
- (BOOL) receivedChanges: (NSArray*)changes errorMessage: (NSString**)errorMessage;
- (BOOL) receivedChange: (NSDictionary*)change;
- (void) stopped; // override this

@end
