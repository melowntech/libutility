#ifndef shared_utility_algorithm_hpp_included_
#define shared_utility_algorithm_hpp_included_

#include <algorithm>

namespace utility { namespace array {

/** Calls unary function op for every item in 2-dimensional array in row major
 *  order. (immutable version)
 */
template <typename T, int rows, int cols, typename UnaryFunction>
UnaryFunction for_each(const T(&data)[rows][cols], UnaryFunction op);

/** Calls unary function op for every item in 2-dimensional array in row major
 *  order. (mutable version)
 */
template <typename T, int rows, int cols, typename UnaryFunction>
UnaryFunction for_each(T(&data)[rows][cols], UnaryFunction op);



// implementation

template <typename T, int rows, int cols, typename UnaryFunction>
inline UnaryFunction for_each(const T(&data)[rows][cols], UnaryFunction op)
{
    return std::for_each(&data[0][0], &data[rows - 1][cols], op);
}

template <typename T, int rows, int cols, typename UnaryFunction>
inline UnaryFunction for_each(T(&data)[rows][cols], UnaryFunction op)
{
    return std::for_each(&data[0][0], &data[rows - 1][cols], op);
}

} } // namespace utility::array

#endif // shared_utility_algorithm_hpp_included_