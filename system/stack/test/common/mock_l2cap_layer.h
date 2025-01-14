/******************************************************************************
 *
 *  Copyright 2018 The Android Open Source Project
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
#pragma once

#include <gmock/gmock.h>

#include <vector>

#include "stack/include/bt_hdr.h"
#include "stack/include/l2cap_interface.h"
#include "types/raw_address.h"

namespace bluetooth {
namespace l2cap {

class L2capInterface {
public:
  virtual uint16_t Register(uint16_t psm, const tL2CAP_APPL_INFO& p_cb_info, bool enable_snoop,
                            tL2CAP_ERTM_INFO* p_ertm_info) = 0;
  virtual uint16_t ConnectRequest(uint16_t psm, const RawAddress& bd_addr) = 0;
  virtual bool ConnectResponse(const RawAddress& bd_addr, uint8_t id, uint16_t lcid,
                               tL2CAP_CONN result, uint16_t status) = 0;
  virtual bool DisconnectRequest(uint16_t cid) = 0;
  virtual bool DisconnectResponse(uint16_t cid) = 0;
  virtual bool ConfigRequest(uint16_t cid, tL2CAP_CFG_INFO* p_cfg) = 0;
  virtual bool ConfigResponse(uint16_t cid, tL2CAP_CFG_INFO* p_cfg) = 0;
  virtual tL2CAP_DW_RESULT DataWrite(uint16_t cid, BT_HDR* p_data) = 0;
  virtual uint16_t RegisterLECoc(uint16_t psm, const tL2CAP_APPL_INFO& cb_info,
                                 uint16_t sec_level) = 0;
  virtual void DeregisterLECoc(uint16_t psm) = 0;
  virtual bool ConnectCreditBasedRsp(const RawAddress& bd_addr, uint8_t id,
                                     std::vector<uint16_t>& lcids, tL2CAP_LE_RESULT_CODE result,
                                     tL2CAP_LE_CFG_INFO* p_cfg) = 0;
  virtual std::vector<uint16_t> ConnectCreditBasedReq(uint16_t psm, const RawAddress& bd_addr,
                                                      tL2CAP_LE_CFG_INFO* p_cfg) = 0;
  virtual bool ReconfigCreditBasedConnsReq(const RawAddress& bd_addr, std::vector<uint16_t>& lcids,
                                           tL2CAP_LE_CFG_INFO* peer_cfg) = 0;
  virtual uint16_t LeCreditDefault() = 0;
  virtual uint16_t LeCreditThreshold() = 0;
  virtual ~L2capInterface() = default;
};

class MockL2capInterface : public L2capInterface {
public:
  MOCK_METHOD4(Register, uint16_t(uint16_t psm, const tL2CAP_APPL_INFO& p_cb_info,
                                  bool enable_snoop, tL2CAP_ERTM_INFO* p_ertm_info));
  MOCK_METHOD2(ConnectRequest, uint16_t(uint16_t psm, const RawAddress& bd_addr));
  MOCK_METHOD5(ConnectResponse, bool(const RawAddress& bd_addr, uint8_t id, uint16_t lcid,
                                     tL2CAP_CONN result, uint16_t status));
  MOCK_METHOD1(DisconnectRequest, bool(uint16_t cid));
  MOCK_METHOD1(DisconnectResponse, bool(uint16_t cid));
  MOCK_METHOD2(ConfigRequest, bool(uint16_t cid, tL2CAP_CFG_INFO* p_cfg));
  MOCK_METHOD2(ConfigResponse, bool(uint16_t cid, tL2CAP_CFG_INFO* p_cfg));
  MOCK_METHOD2(DataWrite, tL2CAP_DW_RESULT(uint16_t cid, BT_HDR* p_data));
  MOCK_METHOD3(RegisterLECoc,
               uint16_t(uint16_t psm, const tL2CAP_APPL_INFO& cb_info, uint16_t sec_level));
  MOCK_METHOD1(DeregisterLECoc, void(uint16_t psm));
  MOCK_METHOD1(GetBleConnRole, uint8_t(const RawAddress& bd_addr));
  MOCK_METHOD5(ConnectCreditBasedRsp,
               bool(const RawAddress& p_bd_addr, uint8_t id, std::vector<uint16_t>& lcids,
                    tL2CAP_LE_RESULT_CODE result, tL2CAP_LE_CFG_INFO* p_cfg));
  MOCK_METHOD3(ConnectCreditBasedReq, std::vector<uint16_t>(uint16_t psm, const RawAddress& bd_addr,
                                                            tL2CAP_LE_CFG_INFO* p_cfg));
  MOCK_METHOD3(ReconfigCreditBasedConnsReq,
               bool(const RawAddress& p_bd_addr, std::vector<uint16_t>& lcids,
                    tL2CAP_LE_CFG_INFO* peer_cfg));
  MOCK_METHOD(uint16_t, LeCreditDefault, ());
  MOCK_METHOD(uint16_t, LeCreditThreshold, ());
};

/**
 * Set the {@link MockL2capInterface} for testing
 *
 * @param mock_l2cap_interface pointer to mock l2cap interface, could be null
 */
void SetMockInterface(MockL2capInterface* mock_l2cap_interface);

}  // namespace l2cap
}  // namespace bluetooth
