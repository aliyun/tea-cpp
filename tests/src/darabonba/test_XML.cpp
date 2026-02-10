#include <darabonba/XML.hpp>
#include <gtest/gtest.h>

using namespace Darabonba;

// ==================== parseXml 测试 ====================
TEST(Darabonba_XML, parseXmlSimple) {
  std::string xml = "<root><name>test</name><value>123</value></root>";
  Json result = XML::parseXml(xml);

  EXPECT_TRUE(result.contains("root"));
  EXPECT_EQ(result["root"]["name"], "test");
  EXPECT_EQ(result["root"]["value"], 123);
}

TEST(Darabonba_XML, parseXmlNested) {
  std::string xml =
      "<root><person><name>Alice</name><age>30</age></person></root>";
  Json result = XML::parseXml(xml);

  EXPECT_TRUE(result.contains("root"));
  EXPECT_EQ(result["root"]["person"]["name"], "Alice");
  EXPECT_EQ(result["root"]["person"]["age"], 30);
}

TEST(Darabonba_XML, parseXmlWithAttributes) {
  std::string xml = R"(<root><item id="1" type="test">hello</item></root>)";
  Json result = XML::parseXml(xml);

  EXPECT_TRUE(result.contains("root"));
  auto item = result["root"]["item"];
  EXPECT_EQ(item["@attributes"]["id"], "1");
  EXPECT_EQ(item["@attributes"]["type"], "test");
}

TEST(Darabonba_XML, parseXmlDuplicateNodes) {
  std::string xml =
      "<root><item>one</item><item>two</item><item>three</item></root>";
  Json result = XML::parseXml(xml);

  EXPECT_TRUE(result["root"]["item"].is_array());
  EXPECT_EQ(result["root"]["item"].size(), 3u);
  EXPECT_EQ(result["root"]["item"][0], "one");
  EXPECT_EQ(result["root"]["item"][1], "two");
  EXPECT_EQ(result["root"]["item"][2], "three");
}

TEST(Darabonba_XML, parseXmlEmpty) {
  std::string xml = "";
  Json result = XML::parseXml(xml);
  EXPECT_TRUE(result.empty());
}

TEST(Darabonba_XML, parseXmlEmptyElement) {
  std::string xml = "<root><empty/></root>";
  Json result = XML::parseXml(xml);

  EXPECT_TRUE(result.contains("root"));
  EXPECT_TRUE(result["root"]["empty"].is_null());
}

TEST(Darabonba_XML, parseXmlNumericValue) {
  std::string xml = "<root><count>42</count><pi>3.14</pi></root>";
  Json result = XML::parseXml(xml);

  EXPECT_EQ(result["root"]["count"], 42);
  EXPECT_DOUBLE_EQ(result["root"]["pi"].get<double>(), 3.14);
}

TEST(Darabonba_XML, parseXmlBooleanValue) {
  std::string xml = "<root><flag>true</flag><other>false</other></root>";
  Json result = XML::parseXml(xml);

  EXPECT_EQ(result["root"]["flag"], true);
  EXPECT_EQ(result["root"]["other"], false);
}

// ==================== toXML 测试 ====================
TEST(Darabonba_XML, toXMLSimple) {
  Json body;
  body["name"] = "test";
  body["value"] = 123;

  std::string xml = XML::toXML(body);

  EXPECT_NE(xml.find("<name>test</name>"), std::string::npos);
  EXPECT_NE(xml.find("<value>123</value>"), std::string::npos);
}

TEST(Darabonba_XML, toXMLNested) {
  Json body;
  body["person"]["name"] = "Alice";
  body["person"]["age"] = 30;

  std::string xml = XML::toXML(body);

  EXPECT_NE(xml.find("<person>"), std::string::npos);
  EXPECT_NE(xml.find("<name>Alice</name>"), std::string::npos);
  EXPECT_NE(xml.find("<age>30</age>"), std::string::npos);
  EXPECT_NE(xml.find("</person>"), std::string::npos);
}

TEST(Darabonba_XML, toXMLArray) {
  Json body;
  body["items"] = Json::array({1, 2, 3});

  std::string xml = XML::toXML(body);

  EXPECT_NE(xml.find("<items>1</items>"), std::string::npos);
  EXPECT_NE(xml.find("<items>2</items>"), std::string::npos);
  EXPECT_NE(xml.find("<items>3</items>"), std::string::npos);
}

TEST(Darabonba_XML, toXMLEmptyObject) {
  Json body = Json::object();
  std::string xml = XML::toXML(body);
  EXPECT_TRUE(xml.empty());
}

TEST(Darabonba_XML, toXMLStringValue) {
  Json body;
  body["msg"] = "hello world";

  std::string xml = XML::toXML(body);
  EXPECT_NE(xml.find("<msg>hello world</msg>"), std::string::npos);
}

TEST(Darabonba_XML, toXMLBooleanValue) {
  Json body;
  body["flag"] = true;

  std::string xml = XML::toXML(body);
  EXPECT_NE(xml.find("<flag>true</flag>"), std::string::npos);
}

// ==================== 往返测试 ====================
TEST(Darabonba_XML, roundTripSimple) {
  Json original;
  original["root"]["name"] = "test";
  original["root"]["count"] = 5;

  std::string xml = "<root>" + XML::toXML(original["root"]) + "</root>";
  Json parsed = XML::parseXml(xml);

  EXPECT_EQ(parsed["root"]["name"], "test");
  EXPECT_EQ(parsed["root"]["count"], 5);
}
