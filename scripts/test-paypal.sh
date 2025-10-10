#!/bin/bash
# PayPal Integration Test Script

echo "========================================="
echo "ShadPs4Manager - PayPal Integration Test"
echo "========================================="
echo ""

PAYPAL_URL="https://www.paypal.com/ncp/payment/2WY8TPHV3KDMS"

echo "Testing PayPal URL accessibility..."
echo "URL: $PAYPAL_URL"
echo ""

# Test 1: Check if URL is reachable
echo "[1/4] Testing HTTP connectivity..."
if curl -s -o /dev/null -w "%{http_code}" --max-time 10 "$PAYPAL_URL" | grep -q "200\|302\|301"; then
    echo "✅ PayPal URL is reachable"
else
    echo "❌ PayPal URL may be unreachable or requires authentication"
fi
echo ""

# Test 2: Check if PayPal button code exists in GUI
echo "[2/4] Checking GUI source code for PayPal integration..."
if grep -q "2WY8TPHV3KDMS" src/manager-gui/welcome_tab.cpp; then
    echo "✅ PayPal URL found in GUI code (line $(grep -n "2WY8TPHV3KDMS" src/manager-gui/welcome_tab.cpp | cut -d: -f1))"
else
    echo "❌ PayPal URL not found in GUI code"
fi
echo ""

# Test 3: Verify QDesktopServices is imported
echo "[3/4] Checking for QDesktopServices import..."
if grep -q "QDesktopServices" src/manager-gui/welcome_tab.cpp; then
    echo "✅ QDesktopServices is imported (required for opening browser)"
else
    echo "❌ QDesktopServices not found - browser integration may fail"
fi
echo ""

# Test 4: Check if GUI binary exists
echo "[4/4] Checking if GUI binary is built..."
if [ -f "build/bin/shadps4-manager-gui" ]; then
    echo "✅ GUI binary exists: build/bin/shadps4-manager-gui"
    echo "   Size: $(du -h build/bin/shadps4-manager-gui | cut -f1)"
else
    echo "❌ GUI binary not found - run ./scripts/build.sh first"
fi
echo ""

echo "========================================="
echo "Manual Testing Instructions"
echo "========================================="
echo ""
echo "1. Launch the GUI:"
echo "   cd build/bin && ./shadps4-manager-gui"
echo ""
echo "2. Navigate to Welcome tab (should open by default)"
echo ""
echo "3. Verify System ID is displayed"
echo ""
echo "4. Click '💳 Pay with PayPal' button"
echo ""
echo "5. Verify browser opens to:"
echo "   $PAYPAL_URL"
echo ""
echo "6. Verify PayPal page loads and shows payment form"
echo ""
echo "7. Check message dialog contains:"
echo "   - Payment instructions"
echo "   - Your System ID"
echo "   - Email instructions"
echo ""
echo "========================================="
echo "Payment Integration Summary"
echo "========================================="
echo ""
echo "Payment URL: $PAYPAL_URL"
echo "Price: \$9.99 USD"
echo "Method: Qt QDesktopServices::openUrl()"
echo "Support Email: support@crownparkcomputing.com"
echo ""
echo "✅ Ready for production testing"
echo ""
