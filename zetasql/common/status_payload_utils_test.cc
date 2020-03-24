//
// Copyright 2019 ZetaSQL Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "zetasql/common/status_payload_utils.h"

#include "zetasql/common/testing/proto_matchers.h"
#include "zetasql/testdata/test_schema.pb.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "zetasql/base/status.h"

namespace zetasql {
namespace internal {
namespace {

using ::zetasql::testing::EqualsProto;
using ::testing::HasSubstr;

std::string StatusPayloadTypeUrl() {
  return GetTypeUrl<zetasql_test::TestStatusPayload>();
}

std::string StatusPayload2TypeUrl() {
  return GetTypeUrl<zetasql_test::TestStatusPayload2>();
}

absl::Cord ToStatusCord(const zetasql_test::TestStatusPayload& p) {
  return absl::Cord(p.SerializeAsString());
}

absl::Cord ToStatusCord(const zetasql_test::TestStatusPayload2& p) {
  return absl::Cord(p.SerializeAsString());
}

}  // namespace

TEST(StatusPayloadUtils, HasPayload) {
  EXPECT_FALSE(HasPayload(zetasql_base::OkStatus()));

  EXPECT_FALSE(HasPayload(
      zetasql_base::Status(zetasql_base::StatusCode::kCancelled, "")));

  zetasql_base::Status ok_status = zetasql_base::OkStatus();
  ok_status.SetPayload(StatusPayloadTypeUrl(),
                       ToStatusCord(zetasql_test::TestStatusPayload()));
  // Ok Status should never have a payload.
  EXPECT_FALSE(HasPayload(ok_status));

  zetasql_base::Status status1 =
      zetasql_base::Status(zetasql_base::StatusCode::kCancelled, "");
  status1.SetPayload(StatusPayloadTypeUrl(),
                     ToStatusCord(zetasql_test::TestStatusPayload()));
  EXPECT_TRUE(HasPayload(status1));
}

TEST(StatusPayloadUtils, GetPayloadCount) {
  EXPECT_EQ(GetPayloadCount(zetasql_base::OkStatus()), 0);

  EXPECT_EQ(GetPayloadCount(
                zetasql_base::Status(zetasql_base::StatusCode::kCancelled, "")),
            0);

  zetasql_base::Status status1 =
      zetasql_base::Status(zetasql_base::StatusCode::kCancelled, "");
  status1.SetPayload(StatusPayloadTypeUrl(),
                     ToStatusCord(zetasql_test::TestStatusPayload()));
  EXPECT_EQ(GetPayloadCount(status1), 1);

  // Attaching the same message twice overwrites, count should be the same.
  zetasql_base::Status status2 =
      zetasql_base::Status(zetasql_base::StatusCode::kCancelled, "");
  status2.SetPayload(StatusPayloadTypeUrl(),
                     ToStatusCord(zetasql_test::TestStatusPayload()));
  status2.SetPayload(StatusPayloadTypeUrl(),
                     ToStatusCord(zetasql_test::TestStatusPayload()));
  EXPECT_EQ(GetPayloadCount(status2), 1);

  // Attaching the same message twice overwrites, count should be the same
  zetasql_base::Status status3 =
      zetasql_base::Status(zetasql_base::StatusCode::kCancelled, "");
  status3.SetPayload(StatusPayloadTypeUrl(),
                     ToStatusCord(zetasql_test::TestStatusPayload()));
  status3.SetPayload(StatusPayload2TypeUrl(),
                     ToStatusCord(zetasql_test::TestStatusPayload2()));
  EXPECT_EQ(GetPayloadCount(status3), 2);
}

TEST(StatusPayloadUtils, HasProto) {
  EXPECT_FALSE(HasPayloadWithType<zetasql_test::TestStatusPayload>(
      zetasql_base::OkStatus()));

  zetasql_base::Status status1 =
      zetasql_base::Status(zetasql_base::StatusCode::kCancelled, "");
  status1.SetPayload(StatusPayloadTypeUrl(),
                     ToStatusCord(zetasql_test::TestStatusPayload()));
  EXPECT_TRUE(HasPayloadWithType<zetasql_test::TestStatusPayload>(status1));
  EXPECT_FALSE(HasPayloadWithType<zetasql_test::TestStatusPayload2>(status1));

  zetasql_base::Status status2 =
      zetasql_base::Status(zetasql_base::StatusCode::kCancelled, "");
  status2.SetPayload(StatusPayloadTypeUrl(),
                     ToStatusCord(zetasql_test::TestStatusPayload()));
  status2.SetPayload(StatusPayload2TypeUrl(),
                     ToStatusCord(zetasql_test::TestStatusPayload2()));
  EXPECT_TRUE(HasPayloadWithType<zetasql_test::TestStatusPayload>(status2));
  EXPECT_TRUE(HasPayloadWithType<zetasql_test::TestStatusPayload2>(status2));
}

TEST(StatusPayloadUtils, GetPayload) {
  // Results undefined, but don't crash.
  GetPayload<zetasql_test::TestStatusPayload>(zetasql_base::OkStatus());

  zetasql_base::Status status1 =
      zetasql_base::Status(zetasql_base::StatusCode::kCancelled, "");
  // Results undefined, but don't crash.
  GetPayload<zetasql_test::TestStatusPayload>(status1);

  zetasql_test::TestStatusPayload status_payload1;
  status_payload1.set_value("msg1");

  zetasql_test::TestStatusPayload2 status_payload2;
  status_payload2.set_f1(15);
  status_payload2.set_f2(12);

  zetasql_base::Status status2 =
      zetasql_base::Status(zetasql_base::StatusCode::kCancelled, "");
  status2.SetPayload(StatusPayloadTypeUrl(), ToStatusCord(status_payload1));
  EXPECT_THAT(GetPayload<zetasql_test::TestStatusPayload>(status2),
              EqualsProto(status_payload1));
  // Results undefined, but don't crash.
  GetPayload<zetasql_test::TestStatusPayload2>(status2);

  zetasql_base::Status status3 =
      zetasql_base::Status(zetasql_base::StatusCode::kCancelled, "");
  status3.SetPayload(StatusPayloadTypeUrl(), ToStatusCord(status_payload1));
  status3.SetPayload(StatusPayload2TypeUrl(), ToStatusCord(status_payload2));
  EXPECT_THAT(
      zetasql::internal::GetPayload<zetasql_test::TestStatusPayload>(status3),
      EqualsProto(status_payload1));
  EXPECT_THAT(zetasql::internal::GetPayload<zetasql_test::TestStatusPayload2>(
                  status3),
              EqualsProto(status_payload2));
}

TEST(StatusPayloadUtils, SetPayload_NoOpForOkStatus) {
  zetasql_base::Status status = zetasql_base::OkStatus();
  internal::AttachPayload(&status, zetasql_test::TestStatusPayload());

  EXPECT_FALSE(HasPayload(status));
}

TEST(StatusPayloadUtils, ErasePayloadTyped_NoOpForOkStatus) {
  zetasql_base::Status status = zetasql_base::OkStatus();
  ErasePayloadTyped<zetasql_test::TestStatusPayload>(&status);
  EXPECT_EQ(status, zetasql_base::OkStatus());
}

TEST(StatusPayloadUtils, ErasePayloadTyped_ClearEverything) {
  zetasql_base::Status status =
      zetasql_base::Status(zetasql_base::StatusCode::kCancelled, "");
  zetasql_test::TestStatusPayload payload;
  payload.set_value("my payload");

  status.SetPayload(StatusPayloadTypeUrl(), ToStatusCord(payload));
  ErasePayloadTyped<zetasql_test::TestStatusPayload>(&status);
  EXPECT_EQ(status,
            zetasql_base::Status(zetasql_base::StatusCode::kCancelled, ""));
  EXPECT_FALSE(HasPayload(status));
}

TEST(StatusPayloadUtils, ErasePayloadTyped_ClearOnlyOneThing) {
  zetasql_test::TestStatusPayload payload1;
  payload1.set_value("my payload");
  zetasql_test::TestStatusPayload2 payload2;
  payload2.set_f1(15);

  zetasql_base::Status status =
      zetasql_base::Status(zetasql_base::StatusCode::kCancelled, "msg");
  status.SetPayload(StatusPayloadTypeUrl(), ToStatusCord(payload1));
  status.SetPayload(StatusPayload2TypeUrl(), ToStatusCord(payload2));

  ErasePayloadTyped<zetasql_test::TestStatusPayload>(&status);

  zetasql_base::Status expected =
      zetasql_base::Status(zetasql_base::StatusCode::kCancelled, "msg");
  expected.SetPayload(StatusPayload2TypeUrl(), ToStatusCord(payload2));
  EXPECT_EQ(status, expected);
}

// Note: we only care about the output of ToString because other tests care
// about it. If we can relax the requirements of other tests in
// zetasql we can relax them here as well.
TEST(StatusPayloadUtils, ToString) {
  EXPECT_EQ(StatusToString(zetasql_base::OkStatus()), "OK");

  std::string cancelled_no_payload_str = StatusToString(
      zetasql_base::Status(zetasql_base::StatusCode::kCancelled, "msg"));
  EXPECT_THAT(cancelled_no_payload_str, HasSubstr("cancelled"));
  EXPECT_THAT(cancelled_no_payload_str, HasSubstr("msg"));

  zetasql_base::Status cancelled_with_payload =
      zetasql_base::Status(zetasql_base::StatusCode::kCancelled, "msg");
  zetasql_test::TestStatusPayload payload1;
  payload1.set_value("my payload");
  zetasql_test::TestStatusPayload2 payload2;
  payload2.set_f1(15);

  internal::AttachPayload(&cancelled_with_payload, payload1);
  internal::AttachPayload(&cancelled_with_payload, payload2);

  std::string cancelled_with_payload_str =
      StatusToString(cancelled_with_payload);

  EXPECT_THAT(cancelled_with_payload_str, HasSubstr("cancelled"));
  EXPECT_THAT(cancelled_with_payload_str, HasSubstr("msg"));
  EXPECT_THAT(cancelled_with_payload_str, HasSubstr(".TestStatusPayload"));
  EXPECT_THAT(cancelled_with_payload_str, HasSubstr("my payload"));
  EXPECT_THAT(cancelled_with_payload_str, HasSubstr("15"));
  EXPECT_THAT(cancelled_with_payload_str, HasSubstr(".TestStatusPayload2"));
}

}  // namespace internal
}  // namespace zetasql
