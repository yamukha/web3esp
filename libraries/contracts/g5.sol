// SPDX-License-Identifier: MIT
pragma solidity ^0.8.17;

import "@openzeppelin/contracts/utils/Strings.sol";

// Creating a contract
contract g5
{    
    uint256 private cnt;
    uint256 private x;
    uint256 private y;

    function get_output() public returns (string memory){
        cnt++;
        return ("Hi, your contract ran successfully");
    }

    function set_output(uint256 amount) public returns (string memory){        
        cnt = amount;
        return (Strings.toString(cnt));
    }
 
    function set_output(uint256 amountx, uint256 amounty ) public returns (string memory){        
        x = amountx;
        y = amounty;
        cnt++;
        return (Strings.toString(cnt));
    }

    function set_string(string memory str) public pure returns (string memory){
        return (str);
    }

     function set_string(string memory str, uint256 amount) public returns (string memory){
        cnt = amount;
        return (str);
    }
}
