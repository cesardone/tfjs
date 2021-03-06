/**
 * @license
 * Copyright 2020 Google LLC. All Rights Reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * =============================================================================
 */

import {registerBackend} from '@tensorflow/tfjs-core';
import {MathBackendCPU} from './base';

// Side effects for default initialization of MathBackendCPU
registerBackend('cpu', () => new MathBackendCPU(), 1 /* priority */);
import './register_all_kernels';

// All exports from this package should be in base.
export * from './base';
