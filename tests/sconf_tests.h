/* Older versions of cmocka does not have this function defined, so
   lets define it so we can run some tests on older OS releases! */
#ifndef assert_float_equal
#include <float.h>

#include <cmocka.h>

/* Function copied from cmocka 1.1.7
   (https://gitlab.com/cmocka/cmocka/-/blob/cmocka-1.1.7/src/cmocka.c#L1120) */
/* Returns 1 if the specified float values are equal, else returns 0. */
static int float_compare(const float left,
                         const float right,
                         const float epsilon) {
    float absLeft;
    float absRight;
    float largest;
    float relDiff;

    float diff = left - right;
    diff = (diff >= 0.f) ? diff : -diff;

    // Check if the numbers are really close -- needed
        // when comparing numbers near zero.
        if (diff <= epsilon) {
            return 1;
    }

    absLeft = (left >= 0.f) ? left : -left;
    absRight = (right >= 0.f) ? right : -right;

    largest = (absRight > absLeft) ? absRight : absLeft;
    relDiff = largest * FLT_EPSILON;

    if (diff > relDiff) {
        return 0;
    }
    return 1;
}

void _assert_float_equal(const float a, const float b, const float epsilon)
{
    const int equal = float_compare(a, b, epsilon);
    if (!equal) {
        fail_msg("%f != %f\n", a, b);
    }
}

#define assert_float_equal(a, b, epsilon) \
    _assert_float_equal((float)a, \
            (float)b, \
            (float)epsilon)

#endif /* assert_float_equal */

