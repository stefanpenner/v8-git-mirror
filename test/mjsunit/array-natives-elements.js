// Copyright 2012 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Flags: --allow-natives-syntax

// IC and Crankshaft support for smi-only elements in dynamic array literals.
function get(foo) { return foo; }  // Used to generate dynamic values.

function array_natives_test() {

  // Ensure small array literals start in specific element kind mode.
  assertTrue(%HasFastSmiElements([]));
  assertTrue(%HasFastSmiElements([1]));
  assertTrue(%HasFastSmiElements([1,2]));
  assertTrue(%HasFastDoubleElements([1.1]));
  assertTrue(%HasFastDoubleElements([1.1,2]));

  // This code exists to eliminate the learning influence of AllocationSites
  // on the following tests.
  var __sequence = 0;
  function make_array_string(literal) {
    this.__sequence = this.__sequence + 1;
    return "/* " + this.__sequence + " */  " + literal;
  }
  function make_array(literal) {
    return eval(make_array_string(literal));
  }

  // Push
  var a0 = make_array("[1, 2, 3]");
  assertTrue(%HasFastSmiElements(a0));
  assertEquals(4, a0.push(4));
  assertTrue(%HasFastSmiElements(a0));
  assertEquals(5, a0.push(1.3));
  assertTrue(%HasFastDoubleElements(a0));
  assertEquals(6, a0.push(1.5));
  assertTrue(%HasFastDoubleElements(a0));
  assertEquals(7, a0.push({}));
  assertTrue(%HasFastObjectElements(a0));
  assertEquals(8, a0.push({}));
  assertTrue(%HasFastObjectElements(a0));
  assertEquals([1,2,3,4,1.3,1.5,{},{}], a0);

  // Concat
  var a1;
  a1 = [1,2,3].concat([]);
  //assertTrue(%HasFastSmiElements(a1));
  assertEquals([1,2,3], a1);
  a1 = [1,2,3].concat([4,5,6]);
  assertTrue(%HasFastSmiElements(a1));
  assertEquals([1,2,3,4,5,6], a1);
  a1 = [1,2,3].concat([4,5,6], [7,8,9]);
  assertTrue(%HasFastSmiElements(a1));
  assertEquals([1,2,3,4,5,6,7,8,9], a1);
  a1 = [1.1,2,3].concat([]);
  assertTrue(%HasFastDoubleElements(a1));
  assertEquals([1.1,2,3], a1);
  a1 = [1,2,3].concat([1.1, 2]);
  assertTrue(%HasFastDoubleElements(a1));
  assertEquals([1,2,3,1.1,2], a1);
  a1 = [1.1,2,3].concat([1, 2]);
  assertTrue(%HasFastDoubleElements(a1));
  assertEquals([1.1,2,3,1,2], a1);
  a1 = [1.1,2,3].concat([1.2, 2]);
  assertTrue(%HasFastDoubleElements(a1));
  assertEquals([1.1,2,3,1.2,2], a1);

  a1 = [1,2,3].concat([{}]);
  assertTrue(%HasFastObjectElements(a1));
  assertEquals([1,2,3,{}], a1);
  a1 = [1.1,2,3].concat([{}]);
  assertTrue(%HasFastObjectElements(a1));
  assertEquals([1.1,2,3,{}], a1);
  a1 = [{}].concat([1,2,3]);
  assertTrue(%HasFastObjectElements(a1));
  assertEquals([{},1,2,3], a1);
  a1 = [{}].concat([1.1,2,3]);
  assertTrue(%HasFastObjectElements(a1));
  assertEquals([{},1.1,2,3], a1);

  // Slice
  var a2 = [1,2,3];
  assertTrue(%HasFastSmiElements(a2.slice()));
  assertTrue(%HasFastSmiElements(a2.slice(1)));
  assertTrue(%HasFastSmiElements(a2.slice(1, 2)));
  assertEquals([1,2,3], a2.slice());
  assertEquals([2,3], a2.slice(1));
  assertEquals([2], a2.slice(1,2));
  a2 = [1.1,2,3];
  assertTrue(%HasFastDoubleElements(a2.slice()));
  assertTrue(%HasFastDoubleElements(a2.slice(1)));
  assertTrue(%HasFastDoubleElements(a2.slice(1, 2)));
  assertEquals([1.1,2,3], a2.slice());
  assertEquals([2,3], a2.slice(1));
  assertEquals([2], a2.slice(1,2));
  a2 = [{},2,3];
  assertTrue(%HasFastObjectElements(a2.slice()));
  assertTrue(%HasFastObjectElements(a2.slice(1)));
  assertTrue(%HasFastObjectElements(a2.slice(1, 2)));
  assertEquals([{},2,3], a2.slice());
  assertEquals([2,3], a2.slice(1));
  assertEquals([2], a2.slice(1,2));

  // Splice
  var a3 = [1,2,3];
  var a3r;
  a3r = a3.splice(0, 0);
  assertTrue(%HasFastSmiElements(a3r));
  assertTrue(%HasFastSmiElements(a3));
  assertEquals([], a3r);
  assertEquals([1, 2, 3], a3);
  a3 = [1,2,3];
  a3r = a3.splice(0, 1);
  assertTrue(%HasFastSmiElements(a3r));
  assertTrue(%HasFastSmiElements(a3));
  assertEquals([1], a3r);
  assertEquals([2, 3], a3);
  a3 = [1,2,3];
  a3r = a3.splice(0, 0, 2);
  assertTrue(%HasFastSmiElements(a3r));
  assertTrue(%HasFastSmiElements(a3));
  assertEquals([], a3r);
  assertEquals([2, 1, 2, 3], a3);
  a3 = [1,2,3];
  a3r = a3.splice(0, 1, 2);
  assertTrue(%HasFastSmiElements(a3r));
  assertTrue(%HasFastSmiElements(a3));
  assertEquals([1], a3r);
  assertEquals([2, 2, 3], a3);
  a3 = [1.1,2,3];
  a3r = a3.splice(0, 0);
  assertTrue(%HasFastDoubleElements(a3r));
  assertTrue(%HasFastDoubleElements(a3));
  assertEquals([], a3r);
  assertEquals([1.1, 2, 3], a3);
  a3 = [1.1, 2, 3];
  a3r = a3.splice(0, 1);
  assertTrue(%HasFastDoubleElements(a3r));
  assertTrue(%HasFastDoubleElements(a3));
  assertEquals([1.1], a3r);
  assertEquals([2, 3], a3);
  a3 = [1.1, 2, 3];
  a3r = a3.splice(0, 0, 2);
  assertTrue(%HasFastDoubleElements(a3r));
  assertTrue(%HasFastDoubleElements(a3));
  assertEquals([], a3r);
  assertEquals([2, 1.1, 2, 3], a3);
  a3 = [1.1, 2, 3];
  assertTrue(%HasFastDoubleElements(a3));
  a3r = a3.splice(0, 1, 2);
  assertTrue(%HasFastDoubleElements(a3r));
  assertTrue(%HasFastDoubleElements(a3));
  assertEquals([1.1], a3r);
  assertEquals([2, 2, 3], a3);
  a3 = [1.1,2,3];
  a3r = a3.splice(0, 0, 2.1);
  assertTrue(%HasFastDoubleElements(a3r));
  assertTrue(%HasFastDoubleElements(a3));
  assertEquals([], a3r);
  assertEquals([2.1, 1.1, 2, 3], a3);
  a3 = [1.1,2,3];
  a3r = a3.splice(0, 1, 2.2);
  assertTrue(%HasFastDoubleElements(a3r));
  assertTrue(%HasFastDoubleElements(a3));
  assertEquals([1.1], a3r);
  assertEquals([2.2, 2, 3], a3);
  a3 = [1,2,3];
  a3r = a3.splice(0, 0, 2.1);
  assertTrue(%HasFastDoubleElements(a3r));
  assertTrue(%HasFastDoubleElements(a3));
  assertEquals([], a3r);
  assertEquals([2.1, 1, 2, 3], a3);
  a3 = [1,2,3];
  a3r = a3.splice(0, 1, 2.2);
  assertTrue(%HasFastDoubleElements(a3r));
  assertTrue(%HasFastDoubleElements(a3));
  assertEquals([1], a3r);
  assertEquals([2.2, 2, 3], a3);
  a3 = [{},2,3];
  a3r = a3.splice(0, 0);
  assertTrue(%HasFastObjectElements(a3r));
  assertTrue(%HasFastObjectElements(a3));
  assertEquals([], a3r);
  assertEquals([{}, 2, 3], a3);
  a3 = [1,2,{}];
  a3r = a3.splice(0, 1);
  assertTrue(%HasFastObjectElements(a3r));
  assertTrue(%HasFastObjectElements(a3));
  assertEquals([1], a3r);
  assertEquals([2, {}], a3);
  a3 = [1,2,3];
  a3r = a3.splice(0, 0, {});
  assertTrue(%HasFastObjectElements(a3r));
  assertTrue(%HasFastObjectElements(a3));
  assertEquals([], a3r);
  assertEquals([{}, 1, 2, 3], a3);
  a3 = [1,2,3];
  a3r = a3.splice(0, 1, {});
  assertTrue(%HasFastObjectElements(a3r));
  assertTrue(%HasFastObjectElements(a3));
  assertEquals([1], a3r);
  assertEquals([{}, 2, 3], a3);
  a3 = [1.1,2,3];
  a3r = a3.splice(0, 0, {});
  assertTrue(%HasFastObjectElements(a3r));
  assertTrue(%HasFastObjectElements(a3));
  assertEquals([], a3r);
  assertEquals([{}, 1.1, 2, 3], a3);
  a3 = [1.1,2,3];
  a3r = a3.splice(0, 1, {});
  assertTrue(%HasFastObjectElements(a3r));
  assertTrue(%HasFastObjectElements(a3));
  assertEquals([1.1], a3r);
  assertEquals([{}, 2, 3], a3);
  a3 = [1.1, 2.2, 3.3];
  a3r = a3.splice(2, 1);
  assertTrue(%HasFastDoubleElements(a3r));
  assertTrue(%HasFastDoubleElements(a3));
  assertEquals([3.3], a3r);
  //assertTrue(%HasFastDoubleElements(a3r));
  assertEquals([1.1, 2.2], a3);
  //assertTrue(%HasFastDoubleElements(a3r));
  a3r = a3.splice(1, 1, 4.4, 5.5);
  //assertTrue(%HasFastDoubleElements(a3r));
  //assertTrue(%HasFastDoubleElements(a3));
  assertEquals([2.2], a3r);
  assertEquals([1.1, 4.4, 5.5], a3);

  // Pop
  var a4 = [1,2,3];
  assertEquals(3, a4.pop());
  assertEquals([1,2], a4);
  //assertTrue(%HasFastSmiElements(a4));
  a4 = [1.1,2,3];
  assertEquals(3, a4.pop());
  assertEquals([1.1,2], a4);
  //assertTrue(%HasFastDoubleElements(a4));
  a4 = [{},2,3];
  assertEquals(3, a4.pop());
  assertEquals([{},2], a4);
  //assertTrue(%HasFastObjectElements(a4));

  // Shift
  var a4 = [1,2,3];
  assertEquals(1, a4.shift());
  assertEquals([2,3], a4);
  //assertTrue(%HasFastSmiElements(a4));
  a4 = [1.1,2,3];
  assertEquals(1.1, a4.shift());
  assertEquals([2,3], a4);
  //assertTrue(%HasFastDoubleElements(a4));
  a4 = [{},2,3];
  assertEquals({}, a4.shift());
  assertEquals([2,3], a4);
  //assertTrue(%HasFastObjectElements(a4));

  // Unshift
  var a4 = [1,2,3];
  assertEquals(4, a4.unshift(1));
  assertTrue(%HasFastSmiElements(a4));
  assertEquals([1,1,2,3], a4);
  a4 = [1,2,3];
  assertEquals(4, a4.unshift(1.1));
  assertTrue(%HasFastDoubleElements(a4));
  assertEquals([1.1,1,2,3], a4);
  a4 = [1.1,2,3];
  assertEquals(4, a4.unshift(1));
  assertTrue(%HasFastDoubleElements(a4));
  assertEquals([1,1.1,2,3], a4);
  a4 = [{},2,3];
  assertEquals(4, a4.unshift(1));
  assertTrue(%HasFastObjectElements(a4));
  assertEquals([1,{},2,3], a4);
  a4 = [{},2,3];
  assertEquals(4, a4.unshift(1.1));
  assertTrue(%HasFastObjectElements(a4));
  assertEquals([1.1,{},2,3], a4);
}

for (var i = 0; i < 3; i++) {
  array_natives_test();
}
%OptimizeFunctionOnNextCall(array_natives_test);
array_natives_test();
