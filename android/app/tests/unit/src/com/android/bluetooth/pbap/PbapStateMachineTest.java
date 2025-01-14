/*
 * Copyright 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.bluetooth.pbap;

import static org.mockito.Mockito.*;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.BluetoothSocket;
import android.os.Handler;
import android.os.HandlerThread;

import androidx.test.filters.MediumTest;
import androidx.test.runner.AndroidJUnit4;

import com.android.bluetooth.TestUtils;
import com.android.bluetooth.btservice.AdapterService;

import org.hamcrest.core.IsInstanceOf;
import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnit;
import org.mockito.junit.MockitoRule;

@MediumTest
@RunWith(AndroidJUnit4.class)
public class PbapStateMachineTest {
    private static final int TEST_NOTIFICATION_ID = 1000000;

    private BluetoothAdapter mAdapter;
    private HandlerThread mHandlerThread;
    private PbapStateMachine mPbapStateMachine;
    private BluetoothDevice mTestDevice;
    private Handler mHandler;
    private BluetoothSocket mSocket;
    private BluetoothPbapService mBluetoothPbapService;
    private boolean mIsAdapterServiceSet;

    @Rule public MockitoRule mockitoRule = MockitoJUnit.rule();

    @Mock private AdapterService mAdapterService;

    @Before
    public void setUp() throws Exception {
        TestUtils.setAdapterService(mAdapterService);
        mIsAdapterServiceSet = true;
        // This line must be called to make sure relevant objects are initialized properly
        mAdapter = BluetoothAdapter.getDefaultAdapter();
        // Get a device for testing
        mTestDevice = mAdapter.getRemoteDevice("00:01:02:03:04:05");

        mHandlerThread = new HandlerThread("PbapTestHandlerThread");
        mHandlerThread.start();
        mHandler = new Handler(mHandlerThread.getLooper());
        mBluetoothPbapService = mock(BluetoothPbapService.class);
        doNothing().when(mBluetoothPbapService).checkOrGetPhonebookPermission(any());
        mPbapStateMachine =
                PbapStateMachine.make(
                        mBluetoothPbapService,
                        mHandlerThread.getLooper(),
                        mTestDevice,
                        mSocket,
                        mHandler,
                        TEST_NOTIFICATION_ID);
    }

    @After
    public void tearDown() throws Exception {
        if (!mIsAdapterServiceSet) {
            return;
        }
        mHandlerThread.quitSafely();
        TestUtils.clearAdapterService(mAdapterService);
    }

    /** Test that initial state is WaitingForAuth */
    @Test
    public void testInitialState() {
        Assert.assertEquals(
                BluetoothProfile.STATE_CONNECTING, mPbapStateMachine.getConnectionState());
        Assert.assertThat(
                mPbapStateMachine.getCurrentState(),
                IsInstanceOf.instanceOf(PbapStateMachine.WaitingForAuth.class));
    }

    /** Test state transition from WaitingForAuth to Finished when the user rejected */
    @Ignore("Class BluetoothSocket is final and cannot be mocked. b/71512958: re-enable it.")
    @Test
    public void testStateTransition_WaitingForAuthToFinished() throws Exception {
        mPbapStateMachine.sendMessage(PbapStateMachine.REJECTED);
        Assert.assertEquals(
                BluetoothProfile.STATE_DISCONNECTED, mPbapStateMachine.getConnectionState());
        Assert.assertThat(
                mPbapStateMachine.getCurrentState(),
                IsInstanceOf.instanceOf(PbapStateMachine.Finished.class));
    }

    /** Test state transition from WaitingForAuth to Finished when the user rejected */
    @Ignore("Class BluetoothSocket is final and cannot be mocked. b/71512958: re-enable it.")
    @Test
    public void testStateTransition_WaitingForAuthToConnected() throws Exception {
        mPbapStateMachine.sendMessage(PbapStateMachine.AUTHORIZED);
        Assert.assertEquals(
                BluetoothProfile.STATE_CONNECTED, mPbapStateMachine.getConnectionState());
        Assert.assertThat(
                mPbapStateMachine.getCurrentState(),
                IsInstanceOf.instanceOf(PbapStateMachine.Connected.class));
    }

    /** Test state transition from Connected to Finished when the OBEX server is done */
    @Ignore("Class BluetoothSocket is final and cannot be mocked. b/71512958: re-enable it.")
    @Test
    public void testStateTransition_ConnectedToFinished() throws Exception {
        mPbapStateMachine.sendMessage(PbapStateMachine.AUTHORIZED);
        Assert.assertEquals(
                BluetoothProfile.STATE_CONNECTED, mPbapStateMachine.getConnectionState());
        Assert.assertThat(
                mPbapStateMachine.getCurrentState(),
                IsInstanceOf.instanceOf(PbapStateMachine.Connected.class));

        // PBAP OBEX transport is done.
        mPbapStateMachine.sendMessage(PbapStateMachine.DISCONNECT);
        Assert.assertEquals(
                BluetoothProfile.STATE_DISCONNECTED, mPbapStateMachine.getConnectionState());
        Assert.assertThat(
                mPbapStateMachine.getCurrentState(),
                IsInstanceOf.instanceOf(PbapStateMachine.Finished.class));
    }
}
