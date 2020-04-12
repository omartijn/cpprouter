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
for registering a member function as the callback. Callbacks may be registered on _any_ class,
as long as they have the correct signature.

### Working with slug data

If the URL patterns contain _slugs_, you are probably interested in the data they hold. To get
access to the data, a special _data transfer object_ must be defined for the callback, and the
callback needs to accept this object as the final parameter.

The _data transfer object_ needs to be annotated so that cpprouter knows how to parse the slug
data, this is done by defining a `using` alias on the class. Assuming that the URL pattern has
two slugs, where the first is purely numeric (i.e. `\d+`) and the second is word data (i.e. `\w+`),
one can define this _data transfer object_ as follows:

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
