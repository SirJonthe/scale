/// @file scale.h
/// @brief A small package designed to work without the use of STL and dynamic memory allocations that can iterate through a source area over a destination area by scaling the source index appropriately. For instance, this can be used to scale data in a source array to fit a destination array. The algorithm supports multi-dimensional index iteration.
/// @author github.com/SirJonthe
/// @date 2025
/// @copyright Public domain.
/// @license CC0 1.0

#ifndef CC0_SCALE_H__
#define CC0_SCALE_H__

#include <cstdint>

namespace cc0
{
	namespace scale
	{
		/// @brief Namespace for internal functions. Do not use these.
		namespace internal
		{
			/// @brief Provides some type information for various integer types of specified bit sizes.
			/// @tparam bits The number of bits to get integer information about.
			template < uint32_t bits > struct intinfo {};

			/// @brief Provides some type information for various integer types of 8-bit width.
			template <>
			struct intinfo<8>
			{
				typedef int8_t      int_t;  // An 8-bit signed type.
				typedef uint8_t     uint_t; // An 8-bit unsigned type.
				typedef intinfo<8>  prev;   // The previous, smaller type info object (there is none, so we alias the same object).
				typedef intinfo<16> next;   // The next, larger type info object for when casting to the next larger type is needed.
			};

			/// @brief Provides some type information for various integer types of 16-bit width.
			template <>
			struct intinfo<16>
			{
				typedef int16_t     int_t;  // An 16-bit signed type.
				typedef uint16_t    uint_t; // An 16-bit unsigned type.
				typedef intinfo<8>  prev;   // The previous, smaller type info object for when casting to the previous smaller type is needed.
				typedef intinfo<32> next;   // The next, larger type info object for when casting to the next larger type is needed.
			};

			/// @brief Provides some type information for various integer types of 32-bit width.
			template <>
			struct intinfo<32>
			{
				typedef int32_t     int_t;  // An 32-bit signed type.
				typedef uint32_t    uint_t; // An 32-bit unsigned type.
				typedef intinfo<16> prev;   // The previous, smaller type info object for when casting to the previous smaller type is needed.
				typedef intinfo<64> next;   // The next, larger type info object for when casting to the next larger type is needed.
			};

			/// @brief Provides some type information for various integer types of 64-bit width.
			template <>
			struct intinfo<64>
			{
				typedef int64_t     int_t;  // An 64-bit signed type.
				typedef uint64_t    uint_t; // An 64-bit unsigned type.
				typedef intinfo<32> prev;   // The previous, smaller type info object for when casting to the previous smaller type is needed.
				typedef intinfo<64> next;   // The next, larger type info object (there is none, so we alias the same object).
			};
		}

		/// @brief A real number with a fixed number of bits dedicated for decimals.
		/// @tparam bits The total number of bits for the base data type. Supported sizes are 8, 16, 32, and 64.
		/// @tparam precision The number of bits dedicated to decimals.
		/// @note 64 bits for base type might very easily lead to overflow when multiplying.
		template < uint32_t bits, uint32_t precision >
		struct fixed
		{
			typename internal::intinfo<bits>::int_t value_bits; // The binary representation of the fixed-point number.

			/// @brief The default constructor. Does nothing, and does not initialize the instance.
			fixed( void ) = default;
			
			/// @brief The copy constructor.
			/// @param NA The instance to copy.
			fixed(const fixed&) = default;
			
			/// @brief The copy operator.
			/// @param NA The instance to copy.
			/// @return The result.
			fixed &operator=(const fixed&) = default;

			/// @brief A conversion constructor that converts an integer into a fixed-point number by upscaling it.
			/// @param n The number to upscale into a fixed-point number.
			fixed(typename internal::intinfo<bits>::int_t n) : value_bits(n << precision) {}

			/// @brief A conversion operator converting the fixed-point number into an integer by downscaling it.
			operator typename internal::intinfo<bits>::int_t( void ) const { return value_bits >> precision; }

			/// @brief Addition.
			/// @param r The right-hand side operator.
			/// @return The result.
			fixed &operator+=(fixed r) { value_bits += r.value_bits; return *this; }

			/// @brief Subtraction.
			/// @param r The right-hand side operator.
			/// @return The result.
			fixed &operator-=(fixed r) { value_bits -= r.value_bits; return *this; }
			
			/// @brief Multiplication.
			/// @param r The right-hand side operator.
			/// @return The result.
			fixed &operator*=(fixed r) {
				typename internal::intinfo<bits>::next::int_t n = typename internal::intinfo<bits>::next::int_t(value_bits) * r.value_bits;
				value_bits = (n >> precision);
				return *this;
			}

			/// @brief Division.
			/// @param r The right-hand side operator.
			/// @return The result.
			fixed &operator/=(fixed r) {
				value_bits = (typename internal::intinfo<bits>::next::int_t(value_bits) << precision) / r.value_bits;
				return *this;
			}
		};

		typedef fixed<32,15> fixed32_t; // The most commonly used fixed-point number format internally.

		/// @brief Creates a new fixed-point number
		/// @param i The integer part.
		/// @param d The fractional part.
		/// @return The fixed-point number.
		fixed32_t fixed32(int16_t i, uint16_t d)
		{
			static constexpr int32_t SCALE = (int32_t(0x7fff) << 15) / 9999;
			fixed32_t f(i);
			if (d < 10)        { d *= 1000; }
			else if (d < 100)  { d *= 100; }
			else if (d < 1000) { d *= 10; }
			f.value_bits += ((int32_t(d) * SCALE) >> 15);
			return f;
		}

		/// @brief A point.
		/// @tparam type_t The base numeric type.
		/// @tparam dimensions The number of dimensions of the point.
		template < typename type_t, uint32_t dimensions >
		struct Point
		{
			type_t e[dimensions]; // The values of the point at each dimension.

			/// @brief Casts the point as a pointer to the underlying type.
			/// @returns A pointer to the underlying type.
			operator type_t*( void ) { return e; }

			/// @brief Casts the point as a pointer to the underlying type.
			/// @returns A pointer to the underlying type.
			operator const type_t*( void ) const { return e; }
		};

		/// @brief An area defined by a start-point and an end-point.
		/// @tparam type_t The base numeric type.
		/// @tparam dimensions The number of dimensions of the area.
		template < typename type_t, uint32_t dimensions >
		struct Area
		{
			Point<type_t,dimensions> a; // The start-point.
			Point<type_t,dimensions> b; // The end-point.
		};

		/// @brief For internal use only. Do not use.
		namespace internal
		{
			/// @brief Swaps the values of two references.
			/// @tparam type_t The type of the references.
			/// @param a The first reference.
			/// @param b The second reference.
			template < typename type_t >
			inline void swap(type_t &a, type_t &b)
			{
				type_t t = a;
				a = b;
				b = t;
			}

			/// @brief Returns the smallest of two values.
			/// @tparam type_t The type of the values.
			/// @param a The first value.
			/// @param b The second value.
			/// @return The smallest of the two parameter values.
			template < typename type_t >
			inline type_t min(type_t a, type_t b)
			{
				return a < b ? a : b;
			}

			/// @brief Returns the greatest of two values.
			/// @tparam type_t The type of the values.
			/// @param a The first value.
			/// @param b The second value.
			/// @return The greatest of the two values.
			template < typename type_t >
			inline type_t max(type_t a, type_t b)
			{
				return a > b ? a : b;
			}

			/// @brief A class used to iterate over multi-dimensional data recursively for each dimension and apply a processing function.
			/// @tparam index The current index of the dimension being iterated over.
			/// @tparam dimensions The number of dimensions to iterate over.
			/// @tparam processor_t The type of the processor function/functor.
			template < uint32_t index, uint32_t dimensions, typename processor_t >
			class iterator
			{
			public:
				/// @brief Recursively iterate over multi-dimensional data and apply a processing function at each scale.
				/// @param dst_index An object containing the index of the destination.
				/// @param src_index An object containing the index of the source.
				/// @param dst_area The area over which to iterate the destination index.
				/// @param src_start The source offset (used for when the destination area was clipped as a result of the destination mask used at a previous stage in processing).
				/// @param src_delta The delta used to iterate through the source index.
				/// @param processor The processor function/functor to apply at each scale.
				void operator()(Point<int32_t,dimensions> &dst_index, Point<fixed32_t,dimensions> &src_index, const Area<int32_t,dimensions> &dst_area, const Point<fixed32_t,dimensions> &src_start, const Point<fixed32_t,dimensions> &src_delta, const processor_t &processor) const
				{
					for (dst_index[index] = dst_area.a[index], src_index[index] = src_start[index]; dst_index[index] < dst_area.b[index]; ++dst_index[index], src_index[index] += src_delta[index]) {
						iterator<index-1,dimensions,processor_t>{}(dst_index, src_index, dst_area, src_start, src_delta, processor);
					}
				}
			};

			/// @brief A class used to iterate over the final dimension of multi-dimensional data and apply a processing function.
			/// @tparam dimensions The number of dimensions to iterate over.
			/// @tparam processor_t The type of the processor function/functor.
			template < uint32_t dimensions, typename processor_t >
			class iterator<0, dimensions, processor_t>
			{
			public:
				/// @brief Iterate over the final dimension in multi-dimensional data and apply a processing function at each scale.
				/// @param dst_index An object containing the index of the destination.
				/// @param src_index An object containing the index of the source.
				/// @param dst_area The area over which to iterate the destination index.
				/// @param src_start The source offset (used for when the destination area was clipped as a result of the destination mask used at a previous stage in processing).
				/// @param src_delta The delta used to iterate through the source index.
				/// @param processor The processor function/functor to apply at each scale.
				void operator()(Point<int32_t,dimensions> &dst_index, Point<fixed32_t,dimensions> &src_index, const Area<int32_t,dimensions> &dst_area, const Point<fixed32_t,dimensions> &src_start, const Point<fixed32_t,dimensions> &src_delta, const processor_t &processor) const
				{
					for (dst_index[0] = dst_area.a[0], src_index[0] = src_start[0]; dst_index[0] < dst_area.b[0]; ++dst_index[0], src_index[0] += src_delta[0]) {
						processor(dst_index, src_index);
					}
				}
			};
		}

		/// @brief Example processor functor that writes memory from one 1D array to another.
		/// @tparam dst_t The type of the destination array.
		/// @tparam src_t The type of the source array.
		template < typename dst_t, typename src_t >
		class write
		{
		private:
			dst_t       *m_dst; // The destination array.
			const src_t *m_src; // The source array.
		
		public:
			/// @brief Creates a new write object.
			/// @param dst The destination array.
			/// @param src The source array.
			write(dst_t *dst, const src_t *src) : m_dst(dst), m_src(src) {}
			
			/// @brief Writes to the destination array from the source array using the provided destination and source indices.
			/// @param dst The destination array index.
			/// @param src The source array index.
			void operator()(const Point<int32_t,1> &dst, const Point<fixed32_t,1> &src)
			{
				m_dst[dst[0]] = dst_t(m_src[int32_t(src[0])]);
			}
		};

		/// @brief Scales a source area across a destination area and applies a processor function. Respects reversed axis sampling when an end point on an axis is less than the start point.
		/// @tparam processor_t The type of the processor function. Will most usefully be a functor containing data to scale.
		/// @tparam dimensions The number of dimensions of the space to iterate over.
		/// @param dst_area The destination area to scale the source area over.
		/// @param src_area The source area to scale over the destination area.
		/// @param processor A function taking a destination index and a source index and performs computations. Can, for instance, be used to scale a source array to fit into a destination array.
		/// @param dst_mask A mask used to discard all processing on the destination buffer that falls outside of the area. Can be used as a way to guard against sampling a destination array outside of accepted bounds, or allow for parallel processing by assigning different cores to different masks on the same destination array. In many cases using the bounds of the destination memory is desired.
		/// @sa write
		template < typename processor_t, uint32_t dimensions >
		void scale(Area<int32_t,dimensions> dst_area, Area<fixed32_t,dimensions> src_area, const processor_t &processor, Area<int32_t,dimensions> dst_mask);
	}
}

/// @brief Addition.
/// @tparam bits The total number of bits for the base data type. Supported sizes are 8, 16, 32, and 64.
/// @tparam precision The number of bits dedicated to decimals.
/// @param l The left-hand side operand.
/// @param r The right-hand side operand.
/// @return The result.
template < uint32_t bits, uint32_t precision > inline cc0::scale::fixed<bits,precision> operator+(cc0::scale::fixed<bits,precision> l, cc0::scale::fixed<bits,precision> r) { return l += r; }

/// @brief Subtraction.
/// @tparam bits The total number of bits for the base data type. Supported sizes are 8, 16, 32, and 64.
/// @tparam precision The number of bits dedicated to decimals.
/// @param l The left-hand side operand.
/// @param r The right-hand side operand.
/// @return The result.
template < uint32_t bits, uint32_t precision > inline cc0::scale::fixed<bits,precision> operator-(cc0::scale::fixed<bits,precision> l, cc0::scale::fixed<bits,precision> r) { return l -= r; }

/// @brief Multiplication.
/// @tparam bits The total number of bits for the base data type. Supported sizes are 8, 16, 32, and 64.
/// @tparam precision The number of bits dedicated to decimals.
/// @param l The left-hand side operand.
/// @param r The right-hand side operand.
/// @return The result.
template < uint32_t bits, uint32_t precision > inline cc0::scale::fixed<bits,precision> operator*(cc0::scale::fixed<bits,precision> l, cc0::scale::fixed<bits,precision> r) { return l *= r; }

/// @brief Division.
/// @tparam bits The total number of bits for the base data type. Supported sizes are 8, 16, 32, and 64.
/// @tparam precision The number of bits dedicated to decimals.
/// @param l The left-hand side operand.
/// @param r The right-hand side operand.
/// @return The result.
template < uint32_t bits, uint32_t precision > inline cc0::scale::fixed<bits,precision> operator/(cc0::scale::fixed<bits,precision> l, cc0::scale::fixed<bits,precision> r) { return l /= r; }

template < typename processor_t, uint32_t dimensions >
void cc0::scale::scale(cc0::scale::Area<int32_t,dimensions> dst_area, cc0::scale::Area<fixed32_t,dimensions> src_area, const processor_t &processor, cc0::scale::Area<int32_t,dimensions> dst_mask)
{
	for (uint32_t i = 0; i < dimensions; ++i) {
		if (dst_mask.a[i] > dst_mask.b[i]) { internal::swap(dst_mask.a[i], dst_mask.b[i]); }
	}

	for (uint32_t i = 0; i < dimensions; ++i) {
		if (dst_area.a[i] == dst_area.b[i]) { return; }
		if (src_area.a[i] == src_area.b[i]) { return; }
	}
	Point<fixed32_t,dimensions> src_delta, src_start;
	for (uint32_t i = 0; i < dimensions; ++i) {
		if (dst_area.a[i] > dst_area.b[i]) {
			internal::swap(dst_area.a[i], dst_area.b[i]);
			internal::swap(src_area.a[i], src_area.b[i]);
		}
		if (dst_area.b[i] < dst_mask.a[i] || dst_area.a[i] >= dst_mask.b[i]) { return; }
		src_delta[i].value_bits = (src_area.b[i].value_bits - src_area.a[i].value_bits) / (dst_area.b[i] - dst_area.a[i]);
		src_start[i] = src_delta[i].value_bits >= 0 ? internal::min(src_area.a[i], src_area.b[i]) : (internal::max(src_area.a[i], src_area.b[i]) + src_delta[i]);
		if (dst_area.a[i] < dst_mask.a[i]) {
			src_start[i].value_bits += src_delta[i].value_bits * (dst_mask.a[i] - dst_area.a[i]);
			dst_area.a[i] = dst_mask.a[i];
		}
		if (dst_area.b[i] >= dst_mask.b[i]) {
			dst_area.b[i] = dst_mask.b[i];
		}
	}
	Point<int32_t,dimensions> dst_index;
	Point<fixed32_t,dimensions> src_index;
	internal::iterator<dimensions-1,dimensions,processor_t>{}(dst_index, src_index, dst_area, src_start, src_delta, processor);
}

#endif
