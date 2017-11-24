#[macro_use]
extern crate lazy_static;

const CMWC_CYCLE: usize = 4096;
const PHI: u32 = 0x9e3779b9;

use std::sync::Mutex;

// Complementary multiply with carry generator
// From http://school.anhb.uwa.edu.au/personalpages/kwessen/shared/Marsaglia03.html
// and https://stackoverflow.com/questions/12884351/complement-multiply-with-carry-cmwc-rng-c-winmain16-compile-issue
// Code review here:
// https://codereview.stackexchange.com/questions/169172/complementary-multiply-with-carry-in-rust/169338

struct ComplementaryMultiplyWithCarryGen {
		q: [u32; CMWC_CYCLE],
		c: u32,
		i: usize,
}

impl ComplementaryMultiplyWithCarryGen {
		fn new(seed: u32) -> ComplementaryMultiplyWithCarryGen {
				let mut q = [0; CMWC_CYCLE];

				q[0] = seed;
				q[1] = seed.wrapping_add(PHI);
				q[2] = seed.wrapping_add(PHI).wrapping_add(PHI);

				for i in 3..CMWC_CYCLE {
						let window = &mut q[i - 3..i + 1];
						window[3] = window[0] ^ window[1] ^ PHI ^ seed;
				}

				ComplementaryMultiplyWithCarryGen {
						q: q,
						c: 362436,
						i: 4095,
				}
		}

		fn reset(&mut self, seed: u32) {
				*self = ComplementaryMultiplyWithCarryGen::new(seed);
		}

		fn random(&mut self) -> u32 {
				const A: u64 = 18782;
				const R: u32 = 0xfffffffe;

				self.i = (self.i + 1) & (CMWC_CYCLE - 1);
				let t = A * self.q[self.i] as u64 + self.c as u64;

				self.c = (t >> 32) as u32;
				let mut x = (t + self.c as u64) as u32;
				if x < self.c {
						x += 1;
						self.c += 1;
				}

				self.q[self.i] = R - x;
				self.q[self.i]
		}
}

// Lazy singleton random number generator
lazy_static!{
		static ref GLOBAL_CMWC_GEN: Mutex<ComplementaryMultiplyWithCarryGen> =
				Mutex::new(ComplementaryMultiplyWithCarryGen::new(0));
}

// Public exports for PointGenLib
// All prefixed with pointgen_ to avoid name conflicts
#[no_mangle]
pub extern "C" fn pointgen_set_cmwc_seed(seed: u32) {
		GLOBAL_CMWC_GEN.lock()
				.expect("Cannot lock to set seed")
				.reset(seed);
}

#[no_mangle]
pub extern "C" fn pointgen_random_cmwc() -> u32 {
		GLOBAL_CMWC_GEN.lock()
				.expect("Cannot lock to produce number")
				.random()
}

#[no_mangle]
pub extern "C" fn pointgen_random_uniform_double() -> f64
{
		(pointgen_random_cmwc() as f64) / (u32::max_value() as f64)
}

#[cfg(test)]
mod test {
		use super::*;

		#[test]
		fn i_didnt_change_anything() {
				let mut rng = ComplementaryMultiplyWithCarryGen::new(0);
				let n: Vec<_> = (0..10).map(|_| rng.random()).collect();
				assert_eq!(n, [4294604858, 367747001, 735501178, 4294962861, 735512785, 3666536092, 3666528614, 4294955383, 367747001, 735501178]);
		}
}