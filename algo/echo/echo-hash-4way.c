#if defined(__AVX512VAES__) && defined(__AVX512F__) && defined(__AVX512VL__) && defined(__AVX512DQ__) && defined(__AVX512BW__)

#include "simd-utils.h"
#include "echo-hash-4way.h"

/*
#include <memory.h>
#include "miner.h"
#include "hash_api.h"
//#include "vperm.h"
#include <immintrin.h>
*/
/*
#ifndef NO_AES_NI
#include <wmmintrin.h>
#else
#include <tmmintrin.h>
#endif
*/

// not used
/*
const unsigned int _k_s0F[] = {0x0F0F0F0F, 0x0F0F0F0F, 0x0F0F0F0F, 0x0F0F0F0F};
const unsigned int _k_ipt[] = {0x5A2A7000, 0xC2B2E898, 0x52227808, 0xCABAE090, 0x317C4D00, 0x4C01307D, 0xB0FDCC81, 0xCD80B1FC};
const unsigned int _k_opt[] = {0xD6B66000, 0xFF9F4929, 0xDEBE6808, 0xF7974121, 0x50BCEC00, 0x01EDBD51, 0xB05C0CE0, 0xE10D5DB1};
const unsigned int _k_inv[] = {0x0D080180, 0x0E05060F, 0x0A0B0C02, 0x04070309, 0x0F0B0780, 0x01040A06, 0x02050809, 0x030D0E0C};
const unsigned int _k_sb1[] = {0xCB503E00, 0xB19BE18F, 0x142AF544, 0xA5DF7A6E, 0xFAE22300, 0x3618D415, 0x0D2ED9EF, 0x3BF7CCC1};
const unsigned int _k_sb2[] = {0x0B712400, 0xE27A93C6, 0xBC982FCD, 0x5EB7E955, 0x0AE12900, 0x69EB8840, 0xAB82234A, 0xC2A163C8};
const unsigned int _k_sb3[] = {0xC0211A00, 0x53E17249, 0xA8B2DA89, 0xFB68933B, 0xF0030A00, 0x5FF35C55, 0xA6ACFAA5, 0xF956AF09};
const unsigned int _k_sb4[] = {0x3FD64100, 0xE1E937A0, 0x49087E9F, 0xA876DE97, 0xC393EA00, 0x3D50AED7, 0x876D2914, 0xBA44FE79};
const unsigned int _k_sb5[] = {0xF4867F00, 0x5072D62F, 0x5D228BDB, 0x0DA9A4F9, 0x3971C900, 0x0B487AC2, 0x8A43F0FB, 0x81B332B8};
const unsigned int _k_sb7[] = {0xFFF75B00, 0xB20845E9, 0xE1BAA416, 0x531E4DAC, 0x3390E000, 0x62A3F282, 0x21C1D3B1, 0x43125170};
const unsigned int _k_sbo[] = {0x6FBDC700, 0xD0D26D17, 0xC502A878, 0x15AABF7A, 0x5FBB6A00, 0xCFE474A5, 0x412B35FA, 0x8E1E90D1};
const unsigned int _k_h63[] = {0x63636363, 0x63636363, 0x63636363, 0x63636363};
const unsigned int _k_hc6[] = {0xc6c6c6c6, 0xc6c6c6c6, 0xc6c6c6c6, 0xc6c6c6c6};
const unsigned int _k_h5b[] = {0x5b5b5b5b, 0x5b5b5b5b, 0x5b5b5b5b, 0x5b5b5b5b};
const unsigned int _k_h4e[] = {0x4e4e4e4e, 0x4e4e4e4e, 0x4e4e4e4e, 0x4e4e4e4e};
const unsigned int _k_h0e[] = {0x0e0e0e0e, 0x0e0e0e0e, 0x0e0e0e0e, 0x0e0e0e0e};
const unsigned int _k_h15[] = {0x15151515, 0x15151515, 0x15151515, 0x15151515};
const unsigned int _k_aesmix1[] = {0x0f0a0500, 0x030e0904, 0x07020d08, 0x0b06010c};
const unsigned int _k_aesmix2[] = {0x000f0a05, 0x04030e09, 0x0807020d, 0x0c0b0601};
const unsigned int _k_aesmix3[] = {0x05000f0a, 0x0904030e, 0x0d080702, 0x010c0b06};
const unsigned int _k_aesmix4[] = {0x0a05000f, 0x0e090403, 0x020d0807, 0x06010c0b};
*/

/*
MYALIGN const unsigned int 	const1[]		= {0x00000001, 0x00000000, 0x00000000, 0x00000000};
MYALIGN const unsigned int	mul2mask[]		= {0x00001b00, 0x00000000, 0x00000000, 0x00000000};
MYALIGN const unsigned int	lsbmask[]		= {0x01010101, 0x01010101, 0x01010101, 0x01010101};
MYALIGN const unsigned int	invshiftrows[]	= {0x070a0d00, 0x0b0e0104, 0x0f020508, 0x0306090c};
MYALIGN const unsigned int	zero[]			= {0x00000000, 0x00000000, 0x00000000, 0x00000000};
*/

MYALIGN const unsigned int	mul2ipt[]		= {0x728efc00, 0x6894e61a, 0x3fc3b14d, 0x25d9ab57, 0xfd5ba600, 0x2a8c71d7, 0x1eb845e3, 0xc96f9234};

// do these need to be reversed?

#define mul2mask \
   m512_const4_32( 0x00001b00, 0, 0, 0 ) 

#define lsbmask    m512_const1_32( 0x01010101 ) 

#define ECHO_SUBBYTES( state, i, j ) \
	state[i][j] = _mm512_aesenc_epi128( state[i][j], k1 ); \
	state[i][j] = _mm512_aesenc_epi128( state[i][j], m512_zero ); \
	k1 = _mm512_add_epi32( k1, m512_one_32 )

#define ECHO_MIXBYTES( state1, state2, j, t1, t2, s2 ) do \
{ \
   const int j1 = ( j+1 ) & 3; \
   const int j2 = ( j+2 ) & 3; \
   const int j3 = ( j+3 ) & 3; \
   s2 = _mm512_add_epi8( state1[ 0 ] [j ], state1[ 0 ][ j ] ); \
	t1 = _mm512_srli_epi16( state1[ 0 ][ j ], 7 ); \
	t1 = _mm512_and_si128( t1, lsbmask );\
	t2 = _mm512_shuffle_epi8( mul2mask, t1 ); \
	s2 = _mm512_xor_si512( s2, t2 ); \
	state2[ 0 ] [j ] = s2; \
	state2[ 1 ] [j ] = state1[ 0 ][ j ]; \
	state2[ 2 ] [j ] = state1[ 0 ][ j ]; \
	state2[ 3 ] [j ] = _mm512_xor_si512( s2, state1[ 0 ][ j ] );\
	s2 = _mm512_add_epi8( state1[ 1 ][ j1 ], state1[ 1 ][ j1 ] ); \
	t1 = _mm512_srli_epi16( state1[ 1 ][ j1 ], 7 ); \
	t1 = _mm512_and_si512( t1, lsbmask ); \
	t2 = _mm512_shuffle_epi8( mul2mask, t1 ); \
	s2 = _mm512_xor_si512( s2, t2 );\
	state2[ 0 ][ j ] = _mm512_xor_si512( state2[ 0 ][ j ], \
                            _mm512_xor_si512( s2, state1[ 1 ][ j1 ] ) ); \
	state2[ 1 ][ j ] = _mm512_xor_si512( state2[ 1 ][ j ], s2 ); \
	state2[ 2 ][ j ] = _mm512_xor_si512( state2[ 2 ][ j ], state1[ 1 ][ j1 ] ); \
	state2[ 3 ][ j ] = _mm512_xor_si512( state2[ 3 ][ j ], state1[ 1 ][ j1 ] ); \
	s2 = _mm512_add_epi8( state1[ 2 ][ j2 ], state1[ 2 ][ j2 ] ); \
	t1 = _mm512_srli_epi16( state1[ 2 ][ j2 ], 7 ); \
	t1 = _mm512_and_si512( t1, lsbmask ); \
	t2 = _mm512_shuffle_epi8( mul2mask, t1 ); \
	s2 = _mm512_xor_si512( s2, t2 ); \
	state2[ 0 ][ j ] = _mm512_xor_si512( state2[ 0 ][ j ], state1[ 2 ][ j2 ] ); \
	state2[ 1 ][ j ] = _mm512_xor_si512( state2[ 1 ][ j ], \
                            _mm512_xor_si512( s2, state1[ 2 ][ j2 ] ) ); \
	state2[ 2 ][ j ] = _mm512_xor_si512128( state2[ 2 ][ j ], s2 ); \
	state2[ 3 ][ j ] = _mm512_xor_si512( state2[ 3][ j ], state1[ 2 ][ j2 ] ); \
	s2 = _mm512_add_epi8( state1[ 3 ][ j3 ], state1[ 3 ][ j3 ] ); \
	t1 = _mm512_srli_epi16( state1[ 3 ][ j3 ], 7 ); \
	t1 = _mm512_and_si512( t1, lsbmask ); \
	t2 = _mm512_shuffle_epi8( mul2mask, t1 ); \
	s2 = _mm512_xor_si512( s2, t2 ); \
	state2[ 0 ][ j ] = _mm512_xor_si512( state2[ 0 ][ j ], state1[ 3 ][ j3 ] ); \
	state2[ 1 ][ j ] = _mm512_xor_si512( state2[ 1 ][ j ], state1[ 3 ][ j3 ] ); \
	state2[ 2 ][ j ] = _mm512_xor_si512( state2[ 2 ][ j ], \
                            _mm512_xor_si512( s2, state1[ 3 ][ j3] ) ); \
	state2[ 3 ][ j ] = _mm512_xor_si512( state2[ 3 ][ j ], s2 )
} while(0)

#define ECHO_ROUND_UNROLL2 \
	ECHO_SUBBYTES(_state, 0, 0);\
	ECHO_SUBBYTES(_state, 1, 0);\
	ECHO_SUBBYTES(_state, 2, 0);\
	ECHO_SUBBYTES(_state, 3, 0);\
	ECHO_SUBBYTES(_state, 0, 1);\
	ECHO_SUBBYTES(_state, 1, 1);\
	ECHO_SUBBYTES(_state, 2, 1);\
	ECHO_SUBBYTES(_state, 3, 1);\
	ECHO_SUBBYTES(_state, 0, 2);\
	ECHO_SUBBYTES(_state, 1, 2);\
	ECHO_SUBBYTES(_state, 2, 2);\
	ECHO_SUBBYTES(_state, 3, 2);\
	ECHO_SUBBYTES(_state, 0, 3);\
	ECHO_SUBBYTES(_state, 1, 3);\
	ECHO_SUBBYTES(_state, 2, 3);\
	ECHO_SUBBYTES(_state, 3, 3);\
	ECHO_MIXBYTES(_state, _state2, 0, t1, t2, s2);\
	ECHO_MIXBYTES(_state, _state2, 1, t1, t2, s2);\
	ECHO_MIXBYTES(_state, _state2, 2, t1, t2, s2);\
	ECHO_MIXBYTES(_state, _state2, 3, t1, t2, s2);\
	ECHO_SUBBYTES(_state2, 0, 0);\
	ECHO_SUBBYTES(_state2, 1, 0);\
	ECHO_SUBBYTES(_state2, 2, 0);\
	ECHO_SUBBYTES(_state2, 3, 0);\
	ECHO_SUBBYTES(_state2, 0, 1);\
	ECHO_SUBBYTES(_state2, 1, 1);\
	ECHO_SUBBYTES(_state2, 2, 1);\
	ECHO_SUBBYTES(_state2, 3, 1);\
	ECHO_SUBBYTES(_state2, 0, 2);\
	ECHO_SUBBYTES(_state2, 1, 2);\
	ECHO_SUBBYTES(_state2, 2, 2);\
	ECHO_SUBBYTES(_state2, 3, 2);\
	ECHO_SUBBYTES(_state2, 0, 3);\
	ECHO_SUBBYTES(_state2, 1, 3);\
	ECHO_SUBBYTES(_state2, 2, 3);\
	ECHO_SUBBYTES(_state2, 3, 3);\
	ECHO_MIXBYTES(_state2, _state, 0, t1, t2, s2);\
	ECHO_MIXBYTES(_state2, _state, 1, t1, t2, s2);\
	ECHO_MIXBYTES(_state2, _state, 2, t1, t2, s2);\
	ECHO_MIXBYTES(_state2, _state, 3, t1, t2, s2)



#define SAVESTATE(dst, src)\
	dst[0][0] = src[0][0];\
	dst[0][1] = src[0][1];\
	dst[0][2] = src[0][2];\
	dst[0][3] = src[0][3];\
	dst[1][0] = src[1][0];\
	dst[1][1] = src[1][1];\
	dst[1][2] = src[1][2];\
	dst[1][3] = src[1][3];\
	dst[2][0] = src[2][0];\
	dst[2][1] = src[2][1];\
	dst[2][2] = src[2][2];\
	dst[2][3] = src[2][3];\
	dst[3][0] = src[3][0];\
	dst[3][1] = src[3][1];\
	dst[3][2] = src[3][2];\
	dst[3][3] = src[3][3]


void echo_4way_compress( echo_4way_context *ctx, const unsigned char *pmsg,
               unsigned int uBlockCount )
{
  unsigned int r, b, i, j;
  __m512i t1, t2, s2, k1;
  __m512i _state[4][4], _state2[4][4], _statebackup[4][4]; 

// unroll   
  for ( i = 0; i < 4; i++ )
  for ( j = 0; j < ctx->uHashSize / 256; j++ )
	 _state[ i ][ j ] = ctx->state[ i ][ j ];

  for ( b = 0; b < uBlockCount; b++ )
  {
    ctx->k = _mm512_add_epi64( ctx->k, ctx->const1536 );

    // load message, make aligned, remove loadu
    for( j = ctx->uHashSize / 256; j < 4; j++ )
    {
      for ( i = 0; i < 4; i++ )
	   {
        _state[ i ][ j ] = _mm512_loadu_si512( 
                     (__m512i*)pmsg + 4 * (j - (ctx->uHashSize / 256)) + i );
	   }
	 }

    // save state
	 SAVESTATE( _statebackup, _state );

	 k1 = ctx->k;

	 for ( r = 0; r < ctx->uRounds / 2; r++ )
	 {
		ECHO_ROUND_UNROLL2;
	 }
		
	 if ( ctx->uHashSize == 256 )
	 {
	   for ( i = 0; i < 4; i++ )
	   {
		   _state[ i ][ 0 ] = _mm512_xor_si512( _state[ i ][ 0 ],
                                              _state[ i ][ 1 ] );
		   _state[ i ][ 0 ] = _mm512_xor_si512( _state[ i ][ 0 ],
                                              _state[ i ][ 2 ] );
		   _state[ i ][ 0 ] = _mm512_xor_si512( _state[ i ][ 0 ],
                                              _state[ i ][ 3 ] );
		   _state[ i ][ 0 ] = _mm512_xor_si512( _state[ i ][ 0 ],
                                              _statebackup[ i ][ 0 ] );
		   _state[ i ][ 0 ] = _mm512_xor_si512( _state[ i ][ 0 ],
                                              _statebackup[ i ][ 1 ] );
		   _state[ i ][ 0 ] = _mm512_xor_si512( _state[ i ][ 0 ],
                                              _statebackup[ i ][ 2 ] ) ;
		   _state[ i ][ 0 ] = _mm512_xor_si512( _state[ i ][ 0 ],
                                              _statebackup[ i ][ 3 ] );
	   }
	 }
	 else
	 {
	   for ( i = 0; i < 4; i++ )
	   {
		   _state[ i ][ 0 ] = _mm512_xor_si512( _state[ i ][ 0 ],
                                              _state[ i ][ 2 ] );
		   _state[ i ][ 1 ] = _mm512_xor_si512( _state[ i ][ 1 ],
                                              _state[ i ][ 3 ] );
		   _state[ i ][ 0 ] = _mm512_xor_si512( _state[ i ][ 0 ],
                                              _statebackup[ i ][ 0 ] );
		   _state[ i ][ 0 ] = _mm512_xor_si512( _state[ i ] [0 ],
                                              _statebackup[ i ][ 2 ] );
		   _state[ i ][ 1 ] = _mm512_xor_si512( _state[ i ][ 1 ],
                                              _statebackup[ i ][ 1 ] );
		   _state[ i ][ 1 ] = _mm512_xor_si512( _state[ i ][ 1 ],
                                              _statebackup[ i ][ 3 ] );
      }
	 }
    pmsg += ctx->uBlockLength;
  }
  SAVESTATE(ctx->state, _state);

}



int echo_4way_init( echo_4way_context *ctx, int nHashSize )
{
	int i, j;

   ctx->k = m512_zero; 
	ctx->processed_bits = 0;
	ctx->uBufferBytes = 0;

	switch( nHashSize )
	{
		case 256:
			ctx->uHashSize = 256;
			ctx->uBlockLength = 192;
			ctx->uRounds = 8;
			ctx->hashsize = _mm512_const4_32( 0, 0, 0, 0x100 );
			ctx->const1536 = _mm512_const4_32( 0, 0, 0, 0x600 );
			break;

		case 512:
			ctx->uHashSize = 512;
			ctx->uBlockLength = 128;
			ctx->uRounds = 10;
			ctx->hashsize = _mm512_const4_32( 0, 0, 0, 0x200 );
			ctx->const1536 = _mm512_const4_32( 0, 0, 0, 0x400);
			break;

		default:
			return BAD_HASHBITLEN;
	}


	for( i = 0; i < 4; i++ )
		for( j = 0; j < nHashSize / 256; j++ )
			ctx->state[ i ][ j ] = ctx->hashsize;

	for( i = 0; i < 4; i++ )
		for( j = nHashSize / 256; j < 4; j++ )
			ctx->state[ i ][ j ] = m512_zero;

	return SUCCESS;
}

int echo_4way_update( echo_4way_context *state, const BitSequence *data, DataLength databitlen )
{
	unsigned int uByteLength, uBlockCount, uRemainingBytes;

	uByteLength = (unsigned int)(databitlen / 8);

	if ( ( state->uBufferBytes + uByteLength ) >= state->uBlockLength )
	{
		if ( state->uBufferBytes != 0 )
		{
			// Fill the buffer
			memcpy( state->buffer + state->uBufferBytes,
               (void*)data, state->uBlockLength - state->uBufferBytes );

			// Process buffer
			echo_4way_compress( state, state->buffer, 1 );
			state->processed_bits += state->uBlockLength * 8;

			data += state->uBlockLength - state->uBufferBytes;
			uByteLength -= state->uBlockLength - state->uBufferBytes;
		}

		// buffer now does not contain any unprocessed bytes

		uBlockCount = uByteLength / state->uBlockLength;
		uRemainingBytes = uByteLength % state->uBlockLength;

		if ( uBlockCount > 0 )
		{
			echo_4way_compress( state, data, uBlockCount );

			state->processed_bits += uBlockCount * state->uBlockLength * 8;
			data += uBlockCount * state->uBlockLength;
		}

		if ( uRemainingBytes > 0 )
		{
			memcpy( state->buffer, (void*)data, uRemainingBytes );
		}

		state->uBufferBytes = uRemainingBytes;
	}
	else
	{
		memcpy( state->buffer + state->uBufferBytes, (void*)data, uByteLength );
		state->uBufferBytes += uByteLength;
	}

	return 0;
}

echo_4way_close( echo_4way_context *state, BitSequence *hashval )
{
	__m512i remainingbits;

	// Add remaining bytes in the buffer
	state->processed_bits += state->uBufferBytes * 8;

	remainingbits = _mm512_set4_epi32( 0, 0, 0, state->uBufferBytes * 8 );

	// Pad with 0x80
	state->buffer[ state->uBufferBytes++ ] = 0x80;
	
	// Enough buffer space for padding in this block?
	if ( ( state->uBlockLength - state->uBufferBytes ) >= 18)
	{
		// Pad with zeros
		memset( state->buffer + state->uBufferBytes, 0,
                         state->uBlockLength - ( state->uBufferBytes + 18 ) );

		// Hash size
		*( (unsigned short*)( state->buffer + state->uBlockLength - 18 ) )
           = state->uHashSize;

		// Processed bits
		*( ( DataLength*)( state->buffer + state->uBlockLength - 16 ) )
           = state->processed_bits;
		*( ( DataLength*)( state->buffer + state->uBlockLength - 8 ) ) = 0;

		// Last block contains message bits?
		if ( state->uBufferBytes == 1 )
		{
			state->k = _mm512_xor_si512( state->k, state->k );
			state->k = _mm512_sub_epi64( state->k, state->const1536 );
		}
		else
		{
			state->k = _mm512_add_epi64( state->k, remainingbits );
			state->k = _mm512_sub_epi64( state->k, state->const1536 );
		}

		// Compress
		echo_4way_compress( state, state->buffer, 1 );
	}
	else
	{
		// Fill with zero and compress
		memset( state->buffer + state->uBufferBytes, 0,
                        state->uBlockLength - state->uBufferBytes );
		state->k = _mm512_add_epi64( state->k, remainingbits );
		state->k = _mm512_sub_epi64( state->k, state->const1536 );
		echo_4way_compress( state, state->buffer, 1 );

		// Last block
		memset( state->buffer, 0, state->uBlockLength - 18 );

		// Hash size
		*( (unsigned short*)( state->buffer + state->uBlockLength - 18 ) )
            = state->uHashSize;

		// Processed bits
		*( (DataLength*)( state->buffer + state->uBlockLength - 16 ) )
            = state->processed_bits;
		*( (DataLength*)( state->buffer + state->uBlockLength - 8 ) ) = 0;

		// Compress the last block
		state->k = _mm512_xor_si512(state->k, state->k);
		state->k = _mm512_sub_epi64(state->k, state->const1536);
		echo_4way_compress(state, state->buffer, 1);
	}

	// Store the hash value
	_mm512_storeu_si512( (__m512i*)hashval + 0, state->state[ 0][ 0 ]);
	_mm512_storeu_si512( (__m512i*)hashval + 1, state->state[ 1][ 0 ]);

	if ( state->uHashSize == 512 )
	{
		_mm512_storeu_si512((__m512i*)hashval + 2, state->state[ 2 ][ 0 ]);
		_mm512_storeu_si512((__m512i*)hashval + 3, state->state[ 3 ][ 0 ]);
	}

	return 0;
}

int echo_4way_update_close( echo_4way_context *state, BitSequence *hashval,
                              const BitSequence *data, DataLength databitlen )
{
  unsigned int uByteLength, uBlockCount, uRemainingBytes;

  uByteLength = (unsigned int)(databitlen / 8);

  if ( (state->uBufferBytes + uByteLength) >= state->uBlockLength )
  {
     if ( state->uBufferBytes != 0 )
     {
        // Fill the buffer
        memcpy( state->buffer + state->uBufferBytes,
                   (void*)data, state->uBlockLength - state->uBufferBytes );

        // Process buffer
        echo_4way_compress( state, state->buffer, 1 );
        state->processed_bits += state->uBlockLength * 8;

        data += state->uBlockLength - state->uBufferBytes;
        uByteLength -= state->uBlockLength - state->uBufferBytes;
     }

     // buffer now does not contain any unprocessed bytes

     uBlockCount = uByteLength / state->uBlockLength;
     uRemainingBytes = uByteLength % state->uBlockLength;

     if ( uBlockCount > 0 )
     {
        echo_4way_compress( state, data, uBlockCount );
        state->processed_bits += uBlockCount * state->uBlockLength * 8;
        data += uBlockCount * state->uBlockLength;
     }

     if ( uRemainingBytes > 0 )
     memcpy(state->buffer, (void*)data, uRemainingBytes);
     state->uBufferBytes = uRemainingBytes;
  }
  else
  {
     memcpy( state->buffer + state->uBufferBytes, (void*)data, uByteLength );
     state->uBufferBytes += uByteLength;
  } 

  __m512i remainingbits;

  // Add remaining bytes in the buffer
  state->processed_bits += state->uBufferBytes * 8;

  remainingbits = _mm512_set4_epi32( 0, 0, 0, state->uBufferBytes * 8 );

  // Pad with 0x80
  state->buffer[ state->uBufferBytes++ ] = 0x80;
  // Enough buffer space for padding in this block?
  if ( (state->uBlockLength - state->uBufferBytes) >= 18 )
   {
     // Pad with zeros
     memset( state->buffer + state->uBufferBytes, 0,i
                        state->uBlockLength - (state->uBufferBytes + 18) );

     // Hash size
     *( (unsigned short*)(state->buffer + state->uBlockLength - 18) )
                   = state->uHashSize;

     // Processed bits
     *( (DataLength*)(state->buffer + state->uBlockLength - 16) ) =
                   state->processed_bits;
     *( (DataLength*)(state->buffer + state->uBlockLength - 8) ) = 0;

     // Last block contains message bits?
     if( state->uBufferBytes == 1 )
     {
        state->k = _mm512_xor_si512( state->k, state->k );
        state->k = _mm512_sub_epi64( state->k, state->const1536 );
     }
     else
     {
        state->k = _mm_add_epi64( state->k, remainingbits );
        state->k = _mm_sub_epi64( state->k, state->const1536 );
     }

     // Compress
     echo_4way_compress( state, state->buffer, 1 );
  }
  else
  {
     // Fill with zero and compress
     memset( state->buffer + state->uBufferBytes, 0,
                state->uBlockLength - state->uBufferBytes );
     state->k = _mm512_add_epi64( state->k, remainingbits );
     state->k = _mm512_sub_epi64( state->k, state->const1536 );
     echo_4way_compress( state, state->buffer, 1 );

     // Last block
     memset( state->buffer, 0, state->uBlockLength - 18 );

     // Hash size
     *( (unsigned short*)(state->buffer + state->uBlockLength - 18) ) =
                 state->uHashSize;

     // Processed bits
     *( (DataLength*)(state->buffer + state->uBlockLength - 16) ) =
                  state->processed_bits;
     *( (DataLength*)(state->buffer + state->uBlockLength - 8) ) = 0;
     // Compress the last block
     state->k = _mm512_xor_si512( state->k, state->k );
     state->k = _mm512_sub_epi64( state->k, state->const1536 );
     echo_4way_compress( state, state->buffer, 1) ;
  }

  // Store the hash value
  _mm512_storeu_si512( (__m512i*)hashval + 0, state->state[ 0 ][ 0] );
  _mm512_storeu_si512( (__m512i*)hashval + 1, state->state[ 1 ][ 0] );

  if ( state->uHashSize == 512 )
  {
     _mm512_storeu_si512( (__m512i*)hashval + 2, state->state[ 2 ][ 0 ] );
     _mm512_storeu_si512( (__m512i*)hashval + 3, state->state[ 3 ][ 0 ] );

  }
  return 0;
}

#endif