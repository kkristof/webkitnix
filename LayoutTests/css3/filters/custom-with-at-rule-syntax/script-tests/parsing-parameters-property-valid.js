// Requires custom-filter-parsing-common.js.

description("Test parameters property parsing in the @-webkit-filter at-rule.");

// These have to be global for the test helpers to see them.
var filterAtRule, styleDeclaration, parametersPropertyValue, subValue;

function testParametersProperty(description, propertyValue, expectedValue, expectedTexts)
{
    var parametersProperty = "parameters: " + propertyValue + ";"
    debug("\n" + description + "\n" + parametersProperty);

    stylesheet.insertRule("@-webkit-filter my-filter { " + parametersProperty + " }", 0);
    filterAtRule = stylesheet.cssRules.item(0);
    shouldBe("filterAtRule.type", "CSSRule.WEBKIT_FILTER_RULE");

    styleDeclaration = filterAtRule.style;
    shouldBe("styleDeclaration.length", "1");
    shouldBe("removeBaseURL(styleDeclaration.getPropertyValue('parameters'))", "\"" + expectedValue + "\"");

    parametersPropertyValue = styleDeclaration.getPropertyCSSValue('parameters');
    shouldHaveConstructor("parametersPropertyValue", "CSSValueList");

    shouldBe("parametersPropertyValue.length", expectedTexts.length.toString()); // shouldBe expects string arguments
  
    if (parametersPropertyValue) {
        for (var i = 0; i < expectedTexts.length; i++) {
            subValue = parametersPropertyValue[i];
            shouldBe("subValue.cssValueType", "CSSValue.CSS_VALUE_LIST");
            shouldBe("removeBaseURL(subValue.cssText)", "\"" + expectedTexts[i] + "\"");
        }
    }
}

heading("Number parameter tests.")
testParametersProperty("Integer parameters.",
    "n1 1, n2 2 3, n3 3 4 5, n4 4 5 6 7",
    "n1 1, n2 2 3, n3 3 4 5, n4 4 5 6 7",
    ["n1 1","n2 2 3", "n3 3 4 5", "n4 4 5 6 7"]);
testParametersProperty("Float parameters.",
    "n1 1.1, n2 2.2 3.3, n3 3.1 4.1 5.1, n4 4.1 5.2 6.3 7.4",
    "n1 1.1, n2 2.2 3.3, n3 3.1 4.1 5.1, n4 4.1 5.2 6.3 7.4",
    ["n1 1.1", "n2 2.2 3.3", "n3 3.1 4.1 5.1", "n4 4.1 5.2 6.3 7.4"]);
testParametersProperty("Parameter name same as CSS keyword.",
    "background 0 1 0 1",
    "background 0 1 0 1",
    ["background 0 1 0 1"]);

heading("Transform parameter tests.")
testParametersProperty("Transform parameter with one transform function.",
    "t rotate(0deg)",
    "t rotate(0deg)",
    ["t rotate(0deg)"]);
testParametersProperty("Transform parameter with multiple transform functions.",
    "t rotate(0deg) perspective(0) scale(0, 0) translate(0px, 0px)",
    "t rotate(0deg) perspective(0) scale(0, 0) translate(0px, 0px)",
    ["t rotate(0deg) perspective(0) scale(0, 0) translate(0px, 0px)"]);
testParametersProperty("Mulitple transform parameters.",
    "t1 rotate(0deg), t2 rotate(0deg)",
    "t1 rotate(0deg), t2 rotate(0deg)",
    ["t1 rotate(0deg)", "t2 rotate(0deg)"]);

heading("Array parameter tests.");
testParametersProperty("Array parameter with name 'array'.",
    "array array(1)",
    "array array(1)",
    ["array array(1)"]);
testParametersProperty("Array parameter with one positive integer.",
    "a array(1)",
    "a array(1)",
    ["a array(1)"]);
testParametersProperty("Array parameter with one negative float.",
    "a array(-1.01)",
    "a array(-1.01)",
    ["a array(-1.01)"]);
testParametersProperty("Array parameter with multiple positive integers.",
    "a array(1, 2, 3, 4, 5)",
    "a array(1, 2, 3, 4, 5)",
    ["a array(1, 2, 3, 4, 5)"]);
testParametersProperty("Array parameter with multiple signed floats.",
    "a array(1, -2.2, 3.14, 0.4, 5)",
    "a array(1, -2.2, 3.14, 0.4, 5)",
    ["a array(1, -2.2, 3.14, 0.4, 5)"]);
testParametersProperty("Multiple array parameters.",
    "a1 array(1, -2.2, 3.14, 0.4, 5), a2 array(1, 2, 3)",
    "a1 array(1, -2.2, 3.14, 0.4, 5), a2 array(1, 2, 3)",
    ["a1 array(1, -2.2, 3.14, 0.4, 5)", "a2 array(1, 2, 3)"]);

heading("Combined parameter tests.");
testParametersProperty("Number parameter, transform parameter, and array parameter.",
    "n 1, t rotate(0deg), a array(1)",
    "n 1, t rotate(0deg), a array(1)",
    ["n 1", "t rotate(0deg)", "a array(1)"]);