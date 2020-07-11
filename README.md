# cpprouter
A modern, header-only request router for C++

![](https://github.com/omartijn/cpprouter/workflows/ubuntu-latest/badge.svg)
![](https://github.com/omartijn/cpprouter/workflows/macos-latest/badge.svg)
![](https://github.com/omartijn/cpprouter/workflows/windows-latest/badge.svg)

## Routing requests

This library is designed to route requests to an associated callback. Requests are matched
using a _pattern_, which may contain embedded regular expressions. The data matched with the
regular expression is then parsed and provided to the callback.

### URL patterns

The router matches incoming requests against list of URL patterns. A URL pattern consists
of a combination of literal data and _slugs_, matched using a regular expression. Slugs are
 opened with a `{` and closed with a `}`.

The following example pattern

`/test/{\d+}/{\w+}`

would match targets like `/test/123/hello` and `/test/999/bye`, but not `/test/xyz/abc` (because
\d only matches digits).

### Callbacks

The library does not impose a particular callback signature on the user, instead letting the user
choose the signature that suits their needs.

The list of patterns and callbacks is stored in a _routing table_, which is available in the
`router::table` class. This class is templated on the signature of the used callback functions.

Say, your callback functions follow the `int(std::string&& body)` signature, you would create
an instance of `router::table<int(std::string&&)>`. On this instance you can register your URL
patterns and their associated callbacks using the `add()` member function:

```
router.add<&callback>("pattern");
router.add<&class::callback>("pattern", instance_pointer);
```

The first variant is used for registering a _free function_, while the second variant is used
for registering a _member function_ as the callback. Callbacks may be registered on _any_ class,
as long as they have the correct signature.

### Working with slug data

If the URL patterns contain _slugs_, you are probably interested in the data they hold. There are
three ways to get this data.

- Accept the data as separate parameters to the callback
- Accept a tuple with the slug data in the callback
- Accept a custom _data transfer object_ in the callback

The first two options are the easiest. Let's assume our previous examples of a callback definition
of `int(std::string&&)` and a pattern of `/test/{\d+}/{\w+}`. From the pattern, it's clear that the
first slug should contain _numeric data_, and the second slug contains a string. We could therefore
define and register our callback like this:

```
int callback(std::string&& body, int numeric, std::string&& word);
int callback(std::string&& body, std::tuple<int, std::string>&& slugs);
```

When a matching request is routed, the slugs are automatically parsed and converted to the right
type before the callback is invoked.

It is also possible to make a dedicated struct, a so-called _data transfer object_, to receive
the slug data.

The _data transfer object_ needs to be annotated so that cpprouter knows how to parse the slug
data, this is done by defining a `using` alias on the class.

```
class slug_dto {
    int numeric;
    std::string word;

    using dto = router::dto<slug_dto>
        ::bind<&slug_dto::numeric>
        ::bind<&slug_dto::word>;
};
```

Assuming the same callback signature as before, the router and callback could be declared as follows:

`int callback(std::string&& body, slug_dto&& slugs);`

### Bringing it all together

See the examples (TODO)
