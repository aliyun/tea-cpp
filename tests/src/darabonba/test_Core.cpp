#include <gtest/gtest.h>
#include <darabonba/Core.hpp>
#include <darabonba/http/Request.hpp>
#include <darabonba/http/MCurlHttpClient.hpp>

using namespace Darabonba;

class CoreTest : public ::testing::Test {
protected:
    Core core;

    virtual void SetUp() override {
        // Setup code here, if needed.
    }

    virtual void TearDown() override {
        // Tear down code here.
    }
};

TEST_F(CoreTest, DoActionReturnsValidResponse) {
    try {
        Http::Request request(std::string("https://example.com/api"));

        request.getHeader()["Content-Type"] = "application/json"; 

        Json runtime;
        runtime["some_key"] = "default_value";

        auto futureResponse = core.doAction(request, runtime);

        std::future_status status = futureResponse.wait_for(std::chrono::seconds(5));
        ASSERT_EQ(status, std::future_status::ready);

        auto response = futureResponse.get();
        ASSERT_NE(response, nullptr);

        // 访问和打印 response 的属性
        std::cout << "Response Status: " << response->getStatusCode() << std::endl;

        // 打印响应头信息
        for (const auto& headerPair : response->getHeaders()) {
            std::cout << headerPair.first << ": " << headerPair.second << std::endl;
        }

        // 打印响应体内容
        std::shared_ptr<Http::MCurlResponseBody> responseBody = response->getBody();
        if (responseBody && responseBody->getReadableSize() > 0) {
            size_t size = responseBody->getReadableSize();
            std::vector<char> buffer(size);
            size_t bytesRead = responseBody->read(buffer.data(), buffer.size());
            std::cout << "Response Body: " << std::string(buffer.data(), bytesRead) << std::endl;
        } else {
            std::cout << "No readable data in response body." << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception occurred in DoActionReturnsValidResponse: " << e.what() << std::endl;
        FAIL() << "Caught exception: " << e.what();
    }
}

TEST_F(CoreTest, DoActionWithEmptyHostFails) {
    try {
        Http::Request request(std::string("/api"));

        Json runtime;
        runtime["some_key"] = "default_value";

        auto futureResponse = core.doAction(request, runtime);

        std::future_status status = futureResponse.wait_for(std::chrono::seconds(5));
        ASSERT_EQ(status, std::future_status::ready);

        auto response = futureResponse.get();
        
        std::cout << "Response Status: " << response->getStatusCode() << std::endl;

        // 打印响应头信息
        for (const auto& headerPair : response->getHeaders()) {
            std::cout << headerPair.first << ": " << headerPair.second << std::endl;
        }

        // 打印响应体内容
        std::shared_ptr<Http::MCurlResponseBody> responseBody = response->getBody();
        if (responseBody && responseBody->getReadableSize() > 0) {
            size_t size = responseBody->getReadableSize();
            std::vector<char> buffer(size);
            size_t bytesRead = responseBody->read(buffer.data(), buffer.size());
            std::cout << "Response Body: " << std::string(buffer.data(), bytesRead) << std::endl;
        } else {
            std::cout << "No readable data in response body." << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception occurred in DoActionWithEmptyHostFails: " << e.what() << std::endl;
        FAIL() << "Caught exception: " << e.what();
    }
}

