--
-- Network protocol, JBP
-- Modified from work by Simon Struck, et al.

-- Declare protocol
jbp_proto = Proto("JBP", "Jaimie's Beun Protocol")
bbp_proto = Proto("BBP", "Beter Beun Protocol")
-- Require JSON parser
local json = require('json')
local offset = 0


function leToNum(buffer, index)
	return ((buffer(offset + index,1):uint() + buffer(offset + index + 1,1):uint() * 256))
end

function leToNum4(buffer, index)
	return (
			(buffer(offset + index + 0,1):uint()) + 
			(buffer(offset + index + 1,1):uint() * 256) + 
			(buffer(offset + index + 2,1):uint() * 256 * 256) + 
			(buffer(offset + index + 3,1):uint() * 256 * 256 * 256)
			)
end


-- create function to dissect
function jbp_proto.dissector(buffer, pinfo, tree)
	offset = 0
    pinfo.cols.protocol = "JBP"
    local sVersion = "JBP "

	proxy = leToNum(buffer, 2)
	destination = leToNum(buffer, 4)

	if proxy == 65535 then proxy = "Broadcast" end
	if destination == 65535 then destination = "Broadcast" end

	while offset < buffer:len() do
		local subtree = tree:add(jbp_proto, buffer(offset, 12 + leToNum(buffer, 8)), "packet " .. leToNum(buffer, 0) .. " --> " .. destination .. " Via " .. proxy)
		
		-- Source Addr
		subtree:add(buffer(offset + 0, 2), "Source Address: " .. leToNum(buffer, 0))
		-- Proxy address
		subtree:add(buffer(offset + 2, 2), "Proxy Address: " ..  proxy)
		-- Destination Addr
		subtree:add(buffer(offset + 4, 2), "Destination Address: " ..  destination)
		
		-- Source Port
		subtree:add(buffer(offset + 6, 2), "Sender nonce: " .. leToNum(buffer, 6))
		-- Payload size
		subtree:add(buffer(offset + 8, 2), "Payload size: " .. leToNum(buffer, 8))
		-- Payload type
		subtree:add(buffer(offset + 10, 1), "Payload type: " .. buffer(offset + 10, 1):uint())
		
		-- Switch according to datatype.
		datatype = buffer(offset + 10, 1):uint()

		
		if datatype == 0 then
			pinfo.cols.info = sVersion .. "Ping packet, node: " .. leToNum(buffer, 0)		
		elseif datatype == 1 then
			entries = leToNum(buffer, 8) / 2
	  		pinfo.cols.info = sVersion .. "Route advertisement(" ..  leToNum(buffer, 0) .. "): " .. entries .. " Routing Entries"
		    local inner = subtree:add(jbp_proto, buffer(offset + 10), "Routes packet")
			local i = 12
		    while leToNum(buffer, 8) - i >= 2 do
		        inner:add(jbp_proto, buffer(offset + i, 2), "Routing Entry: " .. leToNum(buffer, i))
		        i = i + 2
		    end
		
		elseif datatype == 2 then
		    pinfo.cols.info = sVersion .. "BBP " .. offset .. " " .. leToNum(buffer, 8) .. " ".. offset + leToNum(buffer, 8).. " " .. buffer:len()
		    local inner = subtree:add(bbp_proto, buffer(offset + 12), "BBP Packet")
		    inner:add(jbp_proto, buffer(offset + 12, 2), "Source port: " .. leToNum(buffer,12))
		    inner:add(jbp_proto, buffer(offset + 14, 2), "Dest port: " .. leToNum(buffer,14))
		    inner:add(jbp_proto, buffer(offset + 16, 4), "Sequence Num: " .. leToNum4(buffer, 16))
		    inner:add(jbp_proto, buffer(offset + 20, 4), "Acknowledgment Num: " .. leToNum4(buffer, 20))
			inner:add(jbp_proto, buffer(offset + 24, 2), "Window size: " .. leToNum(buffer,24))
		    local flags = inner:add(jbp_proto, buffer(offset + 26, 1), "Flags")
		    flags:add(jbp_proto, buffer(offset + 26, 1), "FYN: " .. buffer(offset + 26, 1):bitfield(7, 1))
		    flags:add(jbp_proto, buffer(offset + 26, 1), "SYN: " .. buffer(offset + 26, 1):bitfield(6, 1))
		    flags:add(jbp_proto, buffer(offset + 26, 1), "RST: " .. buffer(offset + 26, 1):bitfield(5, 1))
		    flags:add(jbp_proto, buffer(offset + 26, 1), "ACK: " .. buffer(offset + 26, 1):bitfield(4, 1))
			--TODO: fix rendering of this string
			if leToNum(buffer, 8) > 26 then
		    	inner:add(jbp_proto, buffer(offset + 28, 1), "Payload: " .. buffer(offset + 28):string())
			end
		
		end
		
		--TODO: fix merged packets
		currentLength =  leToNum(buffer, 8) + 12
		offset = offset + currentLength
		--break
	end
end

-- load the tcp port table
tcp_table = DissectorTable.get("tcp.port")
-- register our protocol to handle tcp port 25565
tcp_table:add(25565, jbp_proto)
